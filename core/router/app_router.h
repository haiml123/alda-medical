#pragma once

#include <functional>
#include <string>

// All application screens/modes
enum class AppMode {
    IDLE,           // Welcome screen
    MONITORING,     // Live EEG view (your current main screen)
    SETTINGS        // Settings dialog (modal style)
};

// Convert mode to string for debugging
inline const char* AppModeToString(AppMode mode) {
    switch (mode) {
        case AppMode::IDLE: return "IDLE";
        case AppMode::MONITORING: return "MONITORING";
        case AppMode::SETTINGS: return "SETTINGS";
        default: return "UNKNOWN";
    }
}

// Simple router for screen navigation
class AppRouter {
public:
    AppRouter() : currentMode(AppMode::IDLE), previousMode(AppMode::IDLE) {}

    // Get current mode
    AppMode getCurrentMode() const { return currentMode; }

    // Transition to new mode
    void transitionTo(AppMode newMode) {
        // Call exit callback for current mode
        if (onModeExit) {
            onModeExit(currentMode);
        }

        // Perform the transition
        previousMode = currentMode;
        currentMode = newMode;

        // Call enter callback for new mode
        if (onModeEnter) {
            onModeEnter(currentMode);
        }
    }

    // Go back to previous mode (useful for modal screens)
    void returnToPreviousMode() {
        transitionTo(previousMode);
    }

    // Callbacks for mode transitions
    std::function<void(AppMode)> onModeEnter;
    std::function<void(AppMode)> onModeExit;

private:
    AppMode currentMode;
    AppMode previousMode;
};