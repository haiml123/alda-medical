#pragma once
#include <string>

namespace elda::models {

    struct Patient {
        std::string subject_id;
        std::string session_id;
        std::time_t recording_date;

        std::string sex;
        int age;

        std::string study_name;
        std::string condition;
        std::string technician;

        std::string notes;

        Patient()
            : sex("X")
            , age(0)
            , recording_date(std::time(nullptr)) {}

        std::string to_edf_patient_field() const {
            std::string field = subject_id + " " + sex + " 01-JAN-1900 Subject";
            field.resize(80, ' ');
            return field;
        }

        std::string to_edf_recording_field() const {
            char date_str[32];
            std::strftime(date_str, sizeof(date_str), "%d-%b-%Y",
                          std::localtime(&recording_date));

            std::string field = "Startdate " + std::string(date_str) +
                                " " + study_name + " " + technician + " NVX136";
            field.resize(80, ' ');
            return field;
        }
    };

}
