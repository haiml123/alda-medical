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
#include "eeg_theme.h"
#include "UI/popup_message/popup_message.h"
#include "UI/toast/toast.h"
#include "views/impedance_viewer/impedance_viewer_screen.h"

using elda::views::impedance_viewer::ImpedanceViewerScreen;
using elda::views::monitoring::MonitoringScreen;

int main() {
    // ========================================================================
    // GLFW + OpenGL Setup
    // ========================================================================
    if (!glfwInit()) {
        std::cerr << "failed to initialize GLFW" << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "ELDA - EEG Acquisition", nullptr, nullptr);
    if (!window) {
        std::cerr << "failed to create GLFW window" << std::endl;
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

    const char* font_path = "fonts/Roboto-Medium.ttf";
    ImFont* merged_font = io.Fonts->AddFontFromFileTTF(font_path, 16.0f, &cfg, greek_range);
    if (!merged_font) {
        std::fprintf(stderr, "[Fonts] could not load %s — Ω may render as '?'\n", font_path);
    }

    // -------------- Init backends after fonts ----------------
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Apply ELDA theme
    ApplyAldaTheme();

    // ========================================================================
    // Initialize Core Application State
    // ========================================================================
    AppState app_state;
    elda::AppStateManager state_manager(app_state);

    std::cout << "[Main] application state initialized" << std::endl;
    std::cout << "[Main] channels: " << CHANNELS << std::endl;
    std::cout << "[Main] sample rate: " << SAMPLE_RATE_HZ << " Hz" << std::endl;

    // ========================================================================
    // Setup Router FIRST (before creating screens)
    // ========================================================================
    AppRouter router;

    // ========================================================================
    // Create Screens
    // ========================================================================
    auto monitoring_screen = std::make_unique<MonitoringScreen>(app_state, state_manager, router);
    auto impedance_screen  = std::make_unique<ImpedanceViewerScreen>(app_state, state_manager, router);

    // ========================================================================
    // Register Screens with Router
    // ========================================================================
    router.register_screen(AppMode::MONITORING, monitoring_screen.get());
    router.register_screen(AppMode::IMPEDANCE_VIEWER, impedance_screen.get());

    // ========================================================================
    // Main Loop
    // ========================================================================
    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // delta_time
        double current_time = glfwGetTime();
        float delta_time = static_cast<float>(current_time - last_time);
        last_time = current_time;

        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ====================================================================
        // Render screen via router
        // ====================================================================
        IScreen* current_screen = router.get_current_screen();
        elda::ui::PopupMessage::instance().render();
        elda::ui::Toast::instance().render();

        if (current_screen) {
            current_screen->update(delta_time);
            current_screen->render();
        } else {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::Begin("No Screen", nullptr,
                         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
            ImGui::SetCursorPos(ImVec2(viewport->Size.x * 0.5f - 100,
                                       viewport->Size.y * 0.5f - 20));
            ImGui::Text("no screen registered for current mode");
            ImGui::End();
        }

        // ====================================================================
        // Global hotkeys
        // ====================================================================
        if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
            bool monitoring = state_manager.is_monitoring();
            auto result = state_manager.set_monitoring(!monitoring);
            if (!result.is_success()) {
                std::fprintf(stderr, "[Main] F5 toggle failed: %s\n", result.message.c_str());
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
            if (state_manager.is_monitoring()) {
                bool recording_active = state_manager.is_recording() && !state_manager.is_paused();
                bool currently_paused = state_manager.is_recording() && state_manager.is_paused();

                if (recording_active) {
                    auto result = state_manager.pause_recording();
                    if (!result.is_success()) {
                        std::fprintf(stderr, "[Main] F7 pause failed: %s\n", result.message.c_str());
                    }
                } else {
                    elda::StateChangeError result;
                    if (currently_paused) {
                        result = state_manager.resume_recording();
                    } else {
                        result = state_manager.start_recording();
                    }

                    if (!result.is_success()) {
                        std::fprintf(stderr, "[Main] F7 record failed: %s\n", result.message.c_str());
                    }
                }
            }
        }

        // ====================================================================
        // Render frame
        // ====================================================================
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
    std::cout << "[Main] shutting down..." << std::endl;

    if (state_manager.is_recording()) {
        state_manager.stop_recording();
    }
    if (state_manager.is_monitoring()) {
        state_manager.set_monitoring(false);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "[Main] clean shutdown complete" << std::endl;
    return 0;
}
