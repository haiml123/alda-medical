#pragma once
#include "imgui.h"
#include "implot.h"
#include "core/core.h"
#include "core/app_state_manager.h"
#include "./views/channels_selector_modal/channels_group_presenter.h"

// Draw header toolbar with safe state management
// Returns its height in pixels
float DrawToolbar(AppState& st, elda::AppStateManager& stateManager);