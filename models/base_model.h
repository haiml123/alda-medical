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
        std::string id;

        virtual ~BaseModel() = default;

        const std::string& get_id() const { return id; }

        const std::string& get_created_at() const { return created_at_; }
        const std::string& get_updated_at() const { return updated_at_; }

        void set_id(const std::string& new_id) { id = new_id; }
        void set_id() { id = generate_unique_id(); }

        void set_created_at(const std::string& timestamp) { created_at_ = timestamp; }
        void set_created_at() { created_at_ = get_current_timestamp(); }

        void set_updated_at(const std::string& timestamp) { updated_at_ = timestamp; }
        void set_updated_at() { updated_at_ = get_current_timestamp(); }

        void on_create() {
            if (id.empty()) {
                set_id();
            }
            if (created_at_.empty()) {
                set_created_at();
            }
            set_updated_at();
        }

        void on_update() {
            set_updated_at();
        }

        bool is_initialized() const {
            return !id.empty() && !created_at_.empty() && !updated_at_.empty();
        }

        static std::string get_current_timestamp() {
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

        static std::string generate_unique_id() {
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
                  const std::string& created_at,
                  const std::string& updated_at)
            : id(id), created_at_(created_at), updated_at_(updated_at) {}

    private:
        std::string created_at_;
        std::string updated_at_;
    };

}
