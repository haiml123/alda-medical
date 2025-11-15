#include "impedance_viewer_header.h"

#include "imgui.h"

namespace elda::views::impedance_viewer {

    void render_impedance_viewer_header(const char* title,
                                     const HeaderCallbacks& callbacks,
                                     float height_px)
    {
        // Chart grid background color (matches ImPlotCol_PlotBg from theme)
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.07f, 0.08f, 1.0f));

        ImGui::BeginChild("impedance_viewer_header",
                          ImVec2(0, height_px),
                          false,
                          ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoScrollWithMouse);

        // Left side: title (bold)
        ImGui::PushFont(ImGui::GetFont());  // same font, just demonstrate customization
        ImGui::TextUnformatted(title);
        ImGui::PopFont();

        // Right side: buttons in a row
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 220.0f);

        if (ImGui::Button("Save", ImVec2(100, height_px - 16.0f))) {
            if (callbacks.on_save) {
                callbacks.on_save();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(100, height_px - 16.0f))) {
            if (callbacks.on_close) {
                callbacks.on_close();
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

} // namespace elda::impedance_viewer::ui
