#pragma once
#include "patient_info.h"
#include <string>
#include <map>
#include <chrono>

namespace elda::models {

    struct Session {
        std::string session_id;
        std::string study_name;

        PatientInfo patient;

        std::map<std::string, float> pre_monitoring_impedance;

        std::chrono::system_clock::time_point session_start_time;

        double sampling_rate;

        bool is_active;

        Session()
            : sampling_rate(5000.0)
            , is_active(false)
        {}

        static bool is_ready_to_monitor() {
            return true;
        }

        std::string generate_filename() const;
    };

}
