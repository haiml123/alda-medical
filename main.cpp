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
#include "core/router/IScreen.h"
#include "views/monitoring/monitoring_screen.h"
#include "views/impedance_viewer/impedance_viewer_screen.h"
#include "eeg_theme.h"
#include "UI/popup_message/popup_message.h"
#include "UI/toast/toast.h"

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

    // -------------- FONT LOADING ----------------
    io.Fonts->AddFontDefault();

    ImFontConfig cfg;
    cfg.MergeMode = true;
    static const ImWchar greek_range[] = { 0x0370, 0x03FF, 0 }; // Greek & Coptic

    const char* fontPath = "fonts/Roboto-Medium.ttf";
    ImFont* merged = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &cfg, greek_range);
    if (!merged) {
        std::fprintf(stderr, "[Fonts] Could not load %s — Ω may render as '?'\n", fontPath);
    }

    // -------------- Init backends after fonts ----------------
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
    // stateManager.EnableAuditLog("elda_audit.log");

    std::cout << "[Main] Application state initialized" << std::endl;
    std::cout << "[Main] Channels: " << CHANNELS << std::endl;
    std::cout << "[Main] Sample Rate: " << SAMPLE_RATE_HZ << " Hz" << std::endl;

    // ========================================================================
    // Setup Router FIRST (before creating screens)
    // ========================================================================
    AppRouter router;

    // ========================================================================
    // Create Screens (pass AppState, StateManager, Router)
    // ========================================================================
    auto monitoringScreen = std::make_unique<elda::MonitoringScreen>(appState, stateManager, router);
    auto impedanceScreen = std::make_unique<elda::impedance_viewer::ImpedanceViewerScreen>(appState, stateManager, router);

    // TODO: Add other screens when ready
    // auto idleScreen = std::make_unique<elda::IdleScreen>(appState, stateManager, router);
    // auto settingsScreen = std::make_unique<elda::SettingsScreen>(appState, stateManager, router);

    // ========================================================================
    // Register Screens with Router
    // ========================================================================
    router.RegisterScreen(AppMode::MONITORING, monitoringScreen.get());
    router.RegisterScreen(AppMode::IMPEDANCE_VIEWER, impedanceScreen.get());

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
        // Route to current screen - SIMPLE!
        // ====================================================================
        IScreen* currentScreen = router.GetCurrentScreen();
        elda::ui::PopupMessage::Instance().Render();
        elda::ui::Toast::Instance().Render();
        if (currentScreen) {
            currentScreen->update(deltaTime);
            currentScreen->render();
        } else {
            // Fallback if no screen registered for current mode
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::Begin("No Screen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
            ImGui::SetCursorPos(ImVec2(viewport->Size.x * 0.5f - 100, viewport->Size.y * 0.5f - 20));
            ImGui::Text("No screen registered for current mode");
            ImGui::End();
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
