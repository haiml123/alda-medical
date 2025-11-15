#pragma once
#include "chart_data.h"
#include "models/channel.h"

void draw_chart(const elda::ChartData& data,
                const std::vector<const elda::models::Channel*>& selected_channels);
