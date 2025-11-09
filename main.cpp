#if defined(__APPLE__)
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl3.h>
#else
  #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <cstdio>

#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "core/core.h"
#include "eeg_theme.h"
#include "UI/toolbar/toolbar.h"
#include "eeg_chart.h"
#include "core/app_state_manager.h"
#include "services/channel_management_service.h"

static void glfw_error_callback(int e, const char* d) {
    std::fprintf(stderr, "GLFW error %d: %s\n", e, d);
}

// SAFE: Static function for observer callback
static const char* GetFieldName(elda::StateField field) {
    static const char* names[] = {
        "Monitoring", "Recording", "Paused", "ChannelConfig",
        "DisplayWindow", "DisplayAmplitude", "NoiseSettings"
    };
    int index = static_cast<int>(field);
    if (index >= 0 && index < 7) {
        return names[index];
    }
    return "Unknown";
}

int main() {
    // ---- GLFW / GL init ----
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1700, 980, "ELDA EEG",
                                         nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

#if !defined(__APPLE__)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to init GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
#endif

    // ---- ImGui / ImPlot ----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 130";
#endif

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ApplyAldaTheme();

    // ---- App state & State Manager ----
    AppState st;
    elda::AppStateManager stateManager(st);

    // Enable audit logging
    stateManager.EnableAuditLog(true, "elda_audit_log.txt");

    // Auto-load active channel group
    auto& channelService = elda::services::ChannelManagementService::GetInstance();
    auto activeGroup = channelService.LoadActiveChannelGroup();

    if (activeGroup.has_value()) {
        auto result = stateManager.SetChannelConfiguration(
            activeGroup->name,
            activeGroup->getSelectedChannels()
        );

        if (result.IsSuccess()) {
            std::printf("[Startup] ✓ Loaded channel group: '%s' with %zu channels\n",
                       activeGroup->name.c_str(), activeGroup->getSelectedCount());

            // DEVELOPMENT MODE: Auto-pass impedance check for synthetic data
            stateManager.SetImpedanceCheckPassed(true);
            std::printf("[Startup] ✓ Impedance check passed (development mode)\n");
            std::printf("[Startup] ✓ Ready to start monitoring!\n");
        }
    } else {
        // DEVELOPMENT MODE: Even without saved channel groups, pass impedance check
        stateManager.SetImpedanceCheckPassed(true);
        std::printf("[Startup] ⚠ No saved channel group - using defaults\n");
        std::printf("[Startup] ✓ Impedance check passed (development mode)\n");
    }

    // Add observer with safe callback
    stateManager.AddObserver([](elda::StateField field) {
        std::printf("[Observer] State changed: %s\n", GetFieldName(field));
        std::fflush(stdout);
    });

    // Synth for data generation
    SynthEEG synth;
    std::vector<float> sample;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ===== HOTKEYS (Original simple logic) =====

        // F5: Toggle Monitoring
        if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
            stateManager.SetMonitoring(!stateManager.IsMonitoring());
        }

        // F7: Toggle Recording/Pause (original combined behavior)
        if (ImGui::IsKeyPressed(ImGuiKey_F7) && stateManager.IsMonitoring()) {
            bool recording = stateManager.IsRecording();
            bool paused = stateManager.IsPaused();

            if (recording && !paused) {
                // Active recording -> pause
                stateManager.PauseRecording();
            } else if (paused) {
                // Paused -> resume
                stateManager.ResumeRecording();
            } else {
                // Not recording -> start
                stateManager.StartRecording();
            }
        }

        // ===== DATA GENERATION (Original logic) =====
        if (stateManager.IsMonitoring()) {
            int n = st.sampler.due();
            for (int i = 0; i < n; ++i) {
                synth.next(sample);
                st.ring.push(sample);
            }
        }

        // ===== CURSOR ADVANCES (Original logic) =====
        st.tickDisplay(stateManager.IsMonitoring());

        // ===== RENDER UI =====
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Full-screen window
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin("Scope", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Toolbar
        Toolbar(st, stateManager);

        // Chart
        DrawChart(st);

        ImGui::End();

        // Render
        ImGui::Render();
        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Shutdown
    std::printf("[Main] Shutting down...\n");

    if (stateManager.IsRecording()) {
        stateManager.StopRecording();
    }
    if (stateManager.IsMonitoring()) {
        stateManager.SetMonitoring(false);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}