// models/session_state.h
#pragma once
#include "patient_info.h"
#include <string>
#include <map>
#include <chrono>

namespace elda::models {

    struct Session {
        // Session identification
        std::string sessionId;
        std::string studyName;

        // Patient information
        PatientInfo patient;

        // Pre-monitoring impedance snapshot (channelId → Ω value)
        std::map<std::string, float> preMonitoringImpedance;

        // Session timing
        std::chrono::system_clock::time_point sessionStartTime;

        // Recording settings
        double samplingRate;

        // Session state flags
        bool isActive;

        Session()
            : samplingRate(5000.0)
            , isActive(false)
        {}

        // Validation helper
        bool isReadyToMonitor() const {
            return true;
        }

        // Utility methods
        std::string generateFilename() const;
    };

} // namespace elda::models