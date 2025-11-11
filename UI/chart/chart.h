#pragma once
#include "chart_data.h"
#include "models/channel.h"  // add

void DrawChart(const elda::ChartData& data,
               const std::vector<const elda::models::Channel*>& selectedChannels);
