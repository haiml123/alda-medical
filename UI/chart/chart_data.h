#ifndef ELDA_CHART_DATA_H
#define ELDA_CHART_DATA_H

#include <vector>

namespace elda
{

/**
 * ChartData - Clean interface for draw_chart
 * This is what draw_chart needs, nothing more
 */
struct ChartData
{
    // Display settings (extracted from your AppState)
    double amplitude_pp_uv;   // From st.ampPPuV()
    double window_seconds;    // From st.windowSec()
    double playhead_seconds;  // From st.playheadSeconds
    double gain_multiplier;   // From st.gainMul()

    int num_channels;
    int sample_rate_hz;
    int buffer_size;

    // Ring buffer (extracted from st.ring)
    struct
    {
        std::vector<double> t_abs;              // Time absolute
        std::vector<std::vector<double>> data;  // [channel][sample]
        int write;
        bool filled;
    } ring;
};

}  // namespace elda

#endif
