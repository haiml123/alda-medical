#pragma once
#include "imgui.h"
#include <string>

namespace elda::views::impedance_viewer {

    ImU32 ChannelColorFromId(const std::string& channelId);

    // Cap outline + grid (ears + nose, thin lines)
    void DrawCapOutline(ImDrawList* dl, const ImVec2& center, float radius);
    void DrawCapGrid(ImDrawList* dl, const ImVec2& center, float radius);

    // Cap-relative coordinate transforms
    ImVec2 CapNormalizedToScreen(const ImVec2& center, float radius, float normX, float normY);
    ImVec2 ScreenToCapNormalizedClamped(const ImVec2& center, float radius, const ImVec2& p);

    // Hit test
    bool PointInCircle(const ImVec2& p, const ImVec2& c, float r);

} // namespace elda::impedance_viewer::helper
