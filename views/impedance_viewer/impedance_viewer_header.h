#pragma once
#include <functional>

namespace elda::views::impedance_viewer
{

struct HeaderCallbacks
{
    std::function<void()> on_save;
    std::function<void()> on_close;
};

void render_impedance_viewer_header(const char* title, const HeaderCallbacks& callbacks, float height_px = 44.0f);

}  // namespace elda::views::impedance_viewer
