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
inline const char* AppModeToString(const AppMode mode) {
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
    AppRouter() : currentMode(AppMode::IMPEDANCE_VIEWER), previousMode(AppMode::IMPEDANCE_VIEWER) {}

    // Register a screen for a specific mode
    void RegisterScreen(AppMode mode, IScreen* screen) {
        screens_[mode] = screen;
    }

    // Get current mode
    AppMode getCurrentMode() const { return currentMode; }

    // Transition to new mode
    void transitionTo(const AppMode newMode) {
        // Exit current screen
        if (const auto current_screen = GetScreen(currentMode)) {
            std::cout << "← Exiting: " << AppModeToString(currentMode) << std::endl;
            current_screen->onExit();
        }

        // Perform the transition
        previousMode = currentMode;
        currentMode = newMode;

        // Enter new screen
        if (const auto newScreen = GetScreen(currentMode)) {
            std::cout << "→ Entering: " << AppModeToString(currentMode) << std::endl;
            newScreen->onEnter();
        }
    }

    // Go back to previous mode (useful for modal screens)
    void returnToPreviousMode() {
        transitionTo(previousMode);
    }

    // Get the screen for a specific mode
    IScreen* GetScreen(const AppMode mode) const {
        const auto it = screens_.find(mode);
        return (it != screens_.end()) ? it->second : nullptr;
    }

    // Get the current active screen
    IScreen* GetCurrentScreen() const {
        return GetScreen(currentMode);
    }

private:
    AppMode currentMode;
    AppMode previousMode;
    std::unordered_map<AppMode, IScreen*> screens_;
};