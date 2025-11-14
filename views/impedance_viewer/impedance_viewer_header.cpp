#include "impedance_viewer_header.h"

#include "imgui.h"

namespace elda::views::impedance_viewer {

    void RenderImpedanceViewerHeader(const char* title,
                                     const HeaderCallbacks& callbacks,
                                     float height_px)
    {
        // Chart grid background color (matches ImPlotCol_PlotBg from theme)
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.07f, 0.08f, 1.0f));

        ImGui::BeginChild("impedance_viewer_header",
                          ImVec2(0, height_px),
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const float padX = 12.0f;
        const ImVec2 btnSize(84.0f, 26.0f);
        const float gap = 8.0f;

        // ---- Left: Title ----
        ImGui::SetCursorPos(ImVec2(padX, (height_px - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::TextUnformatted(title ? title : "Impedance Viewer");

        // ---- Right: Save / Close buttons ----
        const float groupWidth = btnSize.x * 2.0f + gap;
        const float availWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        const float xRight = availWidth - padX - groupWidth;
        const float yCenter = (height_px - btnSize.y) * 0.5f;

        // Position: Save button
        ImGui::SetCursorPos(ImVec2(xRight, yCenter));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.52f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.46f, 0.90f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.18f, 0.52f, 0.98f, 1.0f));
        if (ImGui::Button("Save", btnSize)) {
            if (callbacks.onSave) callbacks.onSave();
        }
        ImGui::PopStyleColor(3);

        // Position: Close button (to the right of Save)
        ImGui::SetCursorPos(ImVec2(xRight + btnSize.x + gap, yCenter));
        if (ImGui::Button("Close", btnSize)) {
            if (callbacks.onClose) callbacks.onClose();
        }

        ImGui::EndChild();
        ImGui::PopStyleColor(); // Pop ChildBg color
    }

} // namespace elda::impedance_viewer::ui