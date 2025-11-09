#include "monitoring_view.h"
#include "imgui.h"
#include "UI/chart/chart.h"

namespace elda {

    void MonitoringView::render(const ChartData& chartData) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin("Monitoring", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Just call the existing DrawChart function!
        DrawChart(chartData);

        ImGui::End();
    }

} // namespace elda