#pragma once
#include <functional>

namespace elda::views::impedance_viewer {

    struct HeaderCallbacks {
        std::function<void()> onSave;
        std::function<void()> onClose;
    };

    void RenderImpedanceViewerHeader(const char* title,
                                     const HeaderCallbacks& callbacks,
                                     float height_px = 44.0f);

}
