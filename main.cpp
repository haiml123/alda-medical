#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "core/router/app_router.h"
#include "views//monitoring/monitoring_screen.h"

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
    ImGui::StyleColorsDark();

    // ========================================================================
    // Create Screens
    // ========================================================================
    auto monitoringScreen = std::make_unique<elda::MonitoringScreen>();

    // TODO: Add other screens when ready
    // auto idleScreen = std::make_unique<elda::IdleScreen>();
    // auto settingsScreen = std::make_unique<elda::SettingsScreen>();

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

    // Start in MONITORING mode
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
                break;

            case AppMode::SETTINGS:
                // Future: settingsScreen->render();
                // Example: if (settingsScreen->shouldClose()) {
                //     router.returnToPreviousMode();
                // }
                break;
        }

        // ====================================================================
        // Example: Switch modes with keyboard (for testing)
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}