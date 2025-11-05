#pragma once
#include "imgui.h"
#include "implot.h"
#include "eeg_core.h"
#include "./screens/channels_selector_modal/channel_selector_modal.h"

// Draw header toolbar; returns its height in pixels
float DrawToolbar(AppState& st);