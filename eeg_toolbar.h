#pragma once
#include "imgui.h"
#include "implot.h"
#include "eeg_core.h"
#include "./views/channels_selector_modal/channels_group_presenter.h"

// Draw header toolbar; returns its height in pixels
float DrawToolbar(AppState& st);