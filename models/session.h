// models/Session.h
#pragma once
#include <string>
#include <ctime>

namespace elda::models {

    // Type: PascalCase
    struct Session {
        // Public members: snake_case (like standard library)
        std::string session_id;
        std::string study_name;
        double sampling_rate;
        bool is_active;

        Session()
            : sampling_rate(5000.0)
            , is_active(false) {}

        // Methods: camelCase
        std::string generateFilename() const;
        bool isRecordingToFile() const { return is_recording_to_file_; }

    private:
        // Private members: snake_case with trailing underscore
        bool is_recording_to_file_;
        std::time_t start_time_;
    };

} // namespace elda::models