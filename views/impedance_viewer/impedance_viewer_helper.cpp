#include "impedance_viewer_helper.h"
#include <algorithm>
#include <cmath>

#include "imgui_internal.h"

namespace elda::impedance_viewer::helper {

static ImU32 kPalette[6] = {
    IM_COL32( 40, 160,  85, 255), // green
    IM_COL32(240, 210,  70, 255), // yellow
    IM_COL32(200,  70,  50, 255), // red
    IM_COL32(230, 230, 230, 255), // white-ish
    IM_COL32( 70,  90, 200, 255), // blue
    IM_COL32(215, 135,  55, 255)  // orange
};

static uint32_t Hash32(const void* data, size_t len) {
    const auto* p = static_cast<const uint8_t*>(data);
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; ++i) { h ^= p[i]; h *= 16777619u; }
    return h;
}

ImU32 ChannelColorFromId(const std::string& channelId) {
    if (channelId.empty()) return IM_COL32(200,200,200,255);
    return kPalette[Hash32(channelId.data(), channelId.size()) % 6];
}

ImVec2 CapNormalizedToScreen(const ImVec2& center, float radius, float x, float y) {
    const float ux = (x - 0.5f) * 2.0f;
    const float uy = (y - 0.5f) * 2.0f;
    return ImVec2(center.x + ux * radius, center.y + uy * radius);
}

ImVec2 ScreenToCapNormalizedClamped(const ImVec2& center, float radius, const ImVec2& p) {
    float vx = (p.x - center.x) / radius;
    float vy = (p.y - center.y) / radius;
    float len = std::sqrt(vx*vx + vy*vy);
    if (len > 1.0f && len > 0.0f) { vx /= len; vy /= len; }
    return ImVec2(0.5f + 0.5f * vx, 0.5f + 0.5f * vy);
}

bool PointInCircle(const ImVec2& p, const ImVec2& c, float r) {
    float dx = p.x - c.x, dy = p.y - c.y;
    return (dx*dx + dy*dy) <= (r*r);
}

// ---- Outline with nose/ears ----
void DrawCapOutline(ImDrawList* dl, const ImVec2& center, float radius) {
    const ImU32 rimCol    = IM_COL32(140, 140, 155, 255);
    const ImU32 accentCol = IM_COL32(140, 140, 155, 255);
    const float rimThick  = 2.0f;
    const float lineThick = 2.0f;

    dl->AddCircle(center, radius, rimCol, 128, rimThick);

    // Nose (outside only)
    const float angHalf = 0.16f;
    const float aL = -IM_PI*0.5f - angHalf;
    const float aR = -IM_PI*0.5f + angHalf;
    ImVec2 nL(center.x + radius * cosf(aL), center.y + radius * sinf(aL));
    ImVec2 nR(center.x + radius * cosf(aR), center.y + radius * sinf(aR));
    const float noseH = std::max(10.0f, radius * 0.10f);
    ImVec2 tip(center.x, center.y - radius - noseH);
    const float cpOut = std::max(6.0f, radius * 0.12f);
    ImVec2 Lc1(nL.x + cpOut*0.25f, nL.y - cpOut*0.90f);
    ImVec2 Lc2(tip.x - cpOut*0.40f, tip.y + cpOut*0.10f);
    ImVec2 Rc1(tip.x + cpOut*0.40f, tip.y + cpOut*0.10f);
    ImVec2 Rc2(nR.x - cpOut*0.25f, nR.y - cpOut*0.90f);
    dl->PathClear();
    dl->PathLineTo(nL);
    dl->PathBezierCubicCurveTo(Lc1, Lc2, tip);
    dl->PathBezierCubicCurveTo(Rc1, Rc2, nR);
    dl->PathStroke(accentCol, 0, lineThick);

    // Ears: single lobe
    const float earSpan  = std::max(24.0f, radius * 0.26f);
    const float earBulge = std::max(14.0f, radius * 0.10f);
    ImVec2 L0(center.x - radius, center.y - earSpan * 0.5f);
    ImVec2 L3(center.x - radius, center.y + earSpan * 0.5f);
    ImVec2 L1(L0.x - earBulge, L0.y + earSpan * 0.18f);
    ImVec2 L2(L3.x - earBulge, L3.y - earSpan * 0.18f);
    dl->AddBezierCubic(L0, L1, L2, L3, accentCol, lineThick);

    ImVec2 R0(center.x + radius, center.y - earSpan * 0.5f);
    ImVec2 R3(center.x + radius, center.y + earSpan * 0.5f);
    ImVec2 R1(R0.x + earBulge, R0.y + earSpan * 0.18f);
    ImVec2 R2(R3.x + earBulge, R3.y - earSpan * 0.18f);
    dl->AddBezierCubic(R0, R1, R2, R3, accentCol, lineThick);
}

void DrawCapGrid(ImDrawList* dl, const ImVec2& center, float radius) {
    const ImU32 c = IM_COL32(60, 60, 70, 120);
    for (int i = 1; i <= 3; ++i) {
        dl->AddCircle(center, radius * (i/3.0f), c, 64, 1.0f);
    }
    for (int i = 0; i < 8; ++i) {
        float a = (i * 2.0f * float(M_PI)) / 8.0f;
        ImVec2 e(center.x + radius * std::cos(a),
                 center.y + radius * std::sin(a));
        dl->AddLine(center, e, c, 1.0f);
    }
}

} // namespace elda::impedance_viewer::helper
