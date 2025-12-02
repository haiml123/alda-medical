#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <iostream>

#include "IScreen.h"

// Forward declare the interface
class IScreen;

// All application screens/modes
enum class AppMode {
    IDLE,
    MONITORING,
    SETTINGS,
    IMPEDANCE_VIEWER
};

// Convert mode to string for debugging
inline const char* app_mode_to_string(const AppMode mode) {
    switch (mode) {
        case AppMode::IDLE: return "IDLE";
        case AppMode::MONITORING: return "MONITORING";
        case AppMode::SETTINGS: return "SETTINGS";
        case AppMode::IMPEDANCE_VIEWER: return "IMPEDANCE_VIEWER";
        default: return "UNKNOWN";
    }
}

// Router for screen navigation with built-in screen management
class AppRouter {
public:
    AppRouter() : current_mode(AppMode::IMPEDANCE_VIEWER), previous_mode(AppMode::IMPEDANCE_VIEWER) {}

    // Register a screen for a specific mode
    void register_screen(AppMode mode, IScreen* screen) {
        screens_[mode] = screen;
    }

    // Get current mode
    AppMode get_current_mode() const { return current_mode; }

    // Transition to new mode
    void transition_to(const AppMode newMode) {
        // Exit current screen
        if (const auto current_screen = get_screen(current_mode)) {
            std::cout << "← Exiting: " << app_mode_to_string(current_mode) << std::endl;
            current_screen->on_exit();
        }

        // Perform the transition
        previous_mode = current_mode;
        current_mode = newMode;

        // Enter new screen
        if (const auto newScreen = get_screen(current_mode)) {
            std::cout << "→ Entering: " << app_mode_to_string(current_mode) << std::endl;
            newScreen->on_enter();
        }
    }

    // Go back to previous mode (useful for modal screens)
    void return_to_previous_mode() {
        transition_to(previous_mode);
    }

    // Get the screen for a specific mode
    IScreen* get_screen(const AppMode mode) const {
        const auto it = screens_.find(mode);
        return (it != screens_.end()) ? it->second : nullptr;
    }

    // Get the current active screen
    IScreen* get_current_screen() const {
        return get_screen(current_mode);
    }

private:
    AppMode current_mode;
    AppMode previous_mode;
    std::unordered_map<AppMode, IScreen*> screens_;
};
