#pragma once
#include "imgui.h"
#include "core/core.h"
#include "core/app_state_manager.h"

// Simple toolbar - no MVP overhead
// Returns toolbar height in pixels
float Toolbar(AppState& st, elda::AppStateManager& stateManager);