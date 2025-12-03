#pragma once

#include "imgui.h"
#include "cap_placement_model.h"
#include "UI/screen_header/screen_header.h"
#include <functional>

namespace elda::views::cap_placement {

    struct CapPlacementViewData {
        const PlacementStep* current_step = nullptr;
        size_t current_index = 0;
        size_t total_steps = 1;
        bool is_first_step = true;
        bool is_last_step = false;
        float animation_progress = 0.0f;  // 0.0 - 1.0 for step animation
    };

    struct CapPlacementViewCallbacks {
        std::function<void()> on_next;
        std::function<void()> on_previous;
        std::function<void()> on_proceed;
        std::function<void()> on_back;
    };

    class CapPlacementView {
    public:
        CapPlacementView() = default;

        void render(const CapPlacementViewData& data,
                    const CapPlacementViewCallbacks& callbacks);

    private:
        void render_content(const CapPlacementViewData& data,
                            const CapPlacementViewCallbacks& callbacks);
        void render_step_indicator(const CapPlacementViewData& data);
        void render_step_content(const CapPlacementViewData& data);
        void render_navigation(const CapPlacementViewData& data,
                               const CapPlacementViewCallbacks& callbacks);
        void render_animation_placeholder(const CapPlacementViewData& data);
    };

} // namespace elda::views::cap_placement