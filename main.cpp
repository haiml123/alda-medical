#define GL_SILENCE_DEPRECATION

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "core/core.h"
#include "core/app_state_manager.h"
#include "core/router/app_router.h"
#include "views/monitoring/monitoring_screen.h"
#include "eeg_theme.h"

int main() {
    // ========================================================================
    // GLFW + OpenGL Setup
    // ========================================================================
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "ELDA - EEG Acquisition", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ========================================================================
    // ImGui Setup
    // ========================================================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Apply ELDA theme
    ApplyAldaTheme();

    // ========================================================================
    // Initialize Core Application State
    // ========================================================================
    AppState appState;
    elda::AppStateManager stateManager(appState);

    // Enable audit logging (optional - for medical device compliance)
    stateManager.EnableAuditLog("elda_audit.log");

    std::cout << "[Main] Application state initialized" << std::endl;
    std::cout << "[Main] Channels: " << CHANNELS << std::endl;
    std::cout << "[Main] Sample Rate: " << SAMPLE_RATE_HZ << " Hz" << std::endl;

    // ========================================================================
    // Create Screens (pass AppState & StateManager)
    // ========================================================================
    auto monitoringScreen = std::make_unique<elda::MonitoringScreen>(appState, stateManager);

    // TODO: Add other screens when ready
    // auto idleScreen = std::make_unique<elda::IdleScreen>(appState, stateManager);
    // auto settingsScreen = std::make_unique<elda::SettingsScreen>(appState, stateManager);
    // auto impedanceScreen = std::make_unique<elda::ImpedanceScreen>(appState, stateManager);

    // ========================================================================
    // Setup Router with Callbacks
    // ========================================================================
    AppRouter router;

    // Called when entering a new mode
    router.onModeEnter = [&](AppMode mode) {
        std::cout << "→ Entering: " << AppModeToString(mode) << std::endl;

        switch (mode) {
            case AppMode::MONITORING:
                monitoringScreen->onEnter();
                break;
            case AppMode::IDLE:
                // Future: idleScreen->onEnter();
                break;
            case AppMode::SETTINGS:
                // Future: settingsScreen->onEnter();
                break;
        }
    };

    // Called when exiting current mode
    router.onModeExit = [&](AppMode mode) {
        std::cout << "← Exiting: " << AppModeToString(mode) << std::endl;

        switch (mode) {
            case AppMode::MONITORING:
                monitoringScreen->onExit();
                break;
            case AppMode::IDLE:
                // Future: idleScreen->onExit();
                break;
            case AppMode::SETTINGS:
                // Future: settingsScreen->onExit();
                break;
        }
    };

    // Start in IDLE mode (proper medical device workflow)
    // User should: Check impedance → Start monitoring → Start recording
    router.transitionTo(AppMode::MONITORING);

    // ========================================================================
    // Main Loop
    // ========================================================================
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Poll events
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ====================================================================
        // Route to current screen
        // ====================================================================
        AppMode mode = router.getCurrentMode();

        switch (mode) {
            case AppMode::MONITORING:
                monitoringScreen->update(deltaTime);
                monitoringScreen->render();
                break;

            case AppMode::IDLE:
                // Future: idleScreen->render();
                // For now, show placeholder
                {
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImGui::SetNextWindowPos(viewport->Pos);
                    ImGui::SetNextWindowSize(viewport->Size);
                    ImGui::Begin("Idle", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
                    ImGui::SetCursorPos(ImVec2(viewport->Size.x * 0.5f - 100, viewport->Size.y * 0.5f - 20));
                    ImGui::Text("IDLE MODE - Press F2 for Monitoring");
                    ImGui::End();
                }
                break;

            case AppMode::SETTINGS:
                // Future: settingsScreen->render();
                // For now, show placeholder
                {
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImGui::SetNextWindowPos(viewport->Pos);
                    ImGui::SetNextWindowSize(viewport->Size);
                    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
                    ImGui::SetCursorPos(ImVec2(viewport->Size.x * 0.5f - 100, viewport->Size.y * 0.5f - 20));
                    ImGui::Text("SETTINGS MODE - Press F1 to return");
                    ImGui::End();
                }
                break;
        }

        // ====================================================================
        // Mode switching with keyboard (for testing)
        // ====================================================================
        if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
            router.transitionTo(AppMode::IDLE);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
            router.transitionTo(AppMode::MONITORING);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F3)) {
            router.transitionTo(AppMode::SETTINGS);
        }

        // ====================================================================
        // Global hotkeys (F5/F7) - delegate to state manager
        // ====================================================================
        if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
            // Toggle monitoring
            bool monitoring = stateManager.IsMonitoring();
            auto result = stateManager.SetMonitoring(!monitoring);
            if (!result.IsSuccess()) {
                std::fprintf(stderr, "[Main] F5 toggle failed: %s\n", result.message.c_str());
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
            // Toggle recording (only if monitoring)
            if (stateManager.IsMonitoring()) {
                bool recordingActive = stateManager.IsRecording() && !stateManager.IsPaused();
                bool currentlyPaused = stateManager.IsRecording() && stateManager.IsPaused();

                if (recordingActive) {
                    // Pause
                    auto result = stateManager.PauseRecording();
                    if (!result.IsSuccess()) {
                        std::fprintf(stderr, "[Main] F7 pause failed: %s\n", result.message.c_str());
                    }
                } else {
                    // Start or resume
                    elda::StateChangeError result;
                    if (currentlyPaused) {
                        result = stateManager.ResumeRecording();
                    } else {
                        result = stateManager.StartRecording();
                    }

                    if (!result.IsSuccess()) {
                        std::fprintf(stderr, "[Main] F7 record failed: %s\n", result.message.c_str());
                    }
                }
            }
        }

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // ========================================================================
    // Cleanup
    // ========================================================================
    std::cout << "[Main] Shutting down..." << std::endl;

    // Properly stop any active recording/monitoring
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

    std::cout << "[Main] Clean shutdown complete" << std::endl;
    return 0;
}