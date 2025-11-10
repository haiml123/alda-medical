#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>

namespace elda::models {

    class BaseModel {
    public:
        // PUBLIC FIELD - direct access
        std::string id;

        virtual ~BaseModel() = default;

        // Getter for backward compatibility
        const std::string& GetId() const { return id; }

        // Timestamp getters
        const std::string& GetCreatedAt() const { return createdAt_; }
        const std::string& GetUpdatedAt() const { return updatedAt_; }

        // Setters
        void SetId(const std::string& newId) { id = newId; }
        void SetId() { id = GenerateUniqueId(); }

        void SetCreatedAt(const std::string& timestamp) { createdAt_ = timestamp; }
        void SetCreatedAt() { createdAt_ = GetCurrentTimestamp(); }

        void SetUpdatedAt(const std::string& timestamp) { updatedAt_ = timestamp; }
        void SetUpdatedAt() { updatedAt_ = GetCurrentTimestamp(); }

        // Lifecycle methods
        void OnCreate() {
            if (id.empty()) {
                SetId();
            }
            if (createdAt_.empty()) {
                SetCreatedAt();
            }
            SetUpdatedAt();
        }

        void OnUpdate() {
            SetUpdatedAt();
        }

        bool IsInitialized() const {
            return !id.empty() && !createdAt_.empty() && !updatedAt_.empty();
        }

        // Static utility methods
        static std::string GetCurrentTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);

            std::tm tm_utc;
            #ifdef _WIN32
                gmtime_s(&tm_utc, &time_t_now);
            #else
                gmtime_r(&time_t_now, &tm_utc);
            #endif

            std::ostringstream oss;
            oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
            return oss.str();
        }

        static std::string GenerateUniqueId() {
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 0xFFFFFF);

            std::ostringstream oss;
            oss << "ent_"
                << std::hex << std::setfill('0') << std::setw(12) << timestamp
                << "_" << std::setw(6) << dis(gen);

            return oss.str();
        }

    protected:
        BaseModel() = default;
        explicit BaseModel(const std::string& id) : id(id) {}
        BaseModel(const std::string& id,
                   const std::string& createdAt,
                   const std::string& updatedAt)
            : id(id), createdAt_(createdAt), updatedAt_(updatedAt) {}

    private:
        std::string createdAt_;
        std::string updatedAt_;
    };

} // namespace elda::models