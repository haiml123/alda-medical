#include "monitoring_view.h"
#include "imgui.h"
#include "UI/chart/chart.h"

namespace elda {

    void MonitoringView::render(const ChartData& chartData,
                               const ToolbarViewModel& toolbarVM,
                               const ToolbarCallbacks& callbacks) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin("Monitoring", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ===== RENDER TOOLBAR AT TOP (using shared component) =====
        MonitoringToolbar(toolbarVM, callbacks);

        // ===== RENDER CHART BELOW TOOLBAR =====
        ImVec2 availableSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("ChartArea", ImVec2(availableSize.x, availableSize.y), false, ImGuiWindowFlags_NoScrollbar);

        DrawChart(chartData);

        ImGui::EndChild();

        ImGui::End();
    }

} // namespace elda