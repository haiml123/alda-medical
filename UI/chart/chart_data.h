#ifndef ELDA_CHART_DATA_H
#define ELDA_CHART_DATA_H

#include <vector>

namespace elda {

    /**
     * ChartData - Clean interface for DrawChart
     * This is what DrawChart needs, nothing more
     */
    struct ChartData {
        // Display settings (extracted from your AppState)
        double amplitudePPuV;       // From st.ampPPuV()
        double windowSeconds;        // From st.windowSec()
        double playheadSeconds;      // From st.playheadSeconds
        double gainMultiplier;       // From st.gainMul()

        int numChannels;
        int sampleRateHz;
        int bufferSize;

        // Ring buffer (extracted from st.ring)
        struct {
            std::vector<double> tAbs;                       // Time absolute
            std::vector<std::vector<double>> data;          // [channel][sample]
            int write;
            bool filled;
        } ring;
    };

} // namespace elda

#endif