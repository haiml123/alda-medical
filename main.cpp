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

#include "eeg_core.h"
#include "eeg_theme.h"
#include "eeg_toolbar.h"
#include "eeg_chart.h"

static void glfw_error_callback(int e, const char* d){ std::fprintf(stderr,"GLFW error %d: %s\n", e, d); }

int main(){
    // ---- GLFW / GL init ----
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1700, 980, "EEG Sweep (ImPlot)", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);

    // VSync (smooth + low CPU).
    glfwSwapInterval(1);
    // glfwSwapInterval(0); // uncap if you want higher FPS cursor

#if !defined(__APPLE__)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr,"Failed to init GLAD\n"); glfwDestroyWindow(window); glfwTerminate(); return 1;
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

    // Theme
    ApplyAldaTheme();

    // ---- App state & synth ----
    AppState st;            // monitoring starts OFF by design now
    SynthEEG synth;
    std::vector<float> sample;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Hotkeys
        if (ImGui::IsKeyPressed(ImGuiKey_F5)) { // toggle Monitor
            if (st.isMonitoring) {
                st.isMonitoring = false;
                if (st.isRecordingToFile) {
                    st.isRecordingToFile = false;
                    st.isPaused = false;
                    // TODO: finalize/close file
                    std::printf("[F5] Monitor OFF; also STOP recording at t=%.3f s\n", st.currentEEGTime());
                } else {
                    std::printf("[F5] Monitor OFF at t=%.3f s\n", st.currentEEGTime());
                }
            } else {
                st.isMonitoring = true;
                std::printf("[F5] Monitor ON at t=%.3f s\n", st.currentEEGTime());
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F7) && st.isMonitoring) { // Record/Pause toggle
            if (st.isRecordingToFile && !st.isPaused) {
                // -> PAUSE
                st.isPaused = true;
                st.pauseMarks.push_back({ st.currentEEGTime() });
                // TODO: pause file writing
                std::printf("[F7] PAUSE at t=%.3f s\n", st.currentEEGTime());
            } else {
                // -> RECORD (start or resume)
                st.isRecordingToFile = true;
                st.isPaused          = false;
                // TODO: start/resume file writing
                std::printf("[F7] %s recording at t=%.3f s\n",
                            "Start/Resume", st.currentEEGTime());
            }
        }

        // Generate samples only while monitoring
        if (st.isMonitoring) {
            int n = st.sampler.due();
            for (int i=0;i<n;++i){ synth.next(sample); st.ring.push(sample); }
        }

        // Cursor/playhead advances only while monitoring
        st.tickDisplay(st.isMonitoring);

        // Frame start
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Full-screen single window
        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin("Scope", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Header / controls
        DrawToolbar(st);

        // Chart (fills rest)
        DrawChart(st);

        ImGui::End();

        // Render
        ImGui::Render();
        int dw, dh; glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
