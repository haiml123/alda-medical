#pragma once
#include <string>

namespace elda::models {
    struct PatientInfo {
        std::string id;
        std::string name;
        std::string dateOfBirth;
        std::string gender;
    };
}