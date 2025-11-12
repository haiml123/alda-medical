#pragma once
#include "imgui.h"
#include <functional>

namespace elda::impedance_viewer::ui {

    struct HeaderCallbacks {
        std::function<void()> onSave;
        std::function<void()> onClose;
    };

    void RenderImpedanceViewerHeader(const char* title,
                                     const HeaderCallbacks& callbacks,
                                     float height_px = 44.0f);

} // namespace elda::impedance_viewer::ui
