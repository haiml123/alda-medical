#pragma once
#include <string>

namespace elda::models {

    struct Patient {
        // Minimal required fields
        std::string subjectId;          // "S001", "P042", etc.
        std::string sessionId;          // "Session_001"
        std::time_t recordingDate;      // Timestamp

        // Optional demographic (if IRB allows)
        std::string sex;                // "M", "F", "X" (default "X")
        int age;                        // Age in years (0 = unknown)

        // Study metadata
        std::string studyName;          // "fMRI_Study_2024"
        std::string condition;          // "Eyes_Open", "Task_A"
        std::string technician;           // Technician name

        // Notes
        std::string notes;              // Free-form session notes

        Patient()
            : sex("X")
            , age(0)
            , recordingDate(std::time(nullptr)) {}

        // Convert to EDF+ format
        std::string ToEDFPatientField() const {
            std::string field = subjectId + " " + sex + " 01-JAN-1900 Subject";
            field.resize(80, ' ');
            return field;
        }

        std::string ToEDFRecordingField() const {
            char dateStr[32];
            std::strftime(dateStr, sizeof(dateStr), "%d-%b-%Y",
                         std::localtime(&recordingDate));

            std::string field = "Startdate " + std::string(dateStr) +
                               " " + studyName + " " + technician + " NVX136";
            field.resize(80, ' ');
            return field;
        }
    };

} // namespace elda::models