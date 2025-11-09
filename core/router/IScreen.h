#pragma once

// Base interface for all screens
class IScreen {
public:
    virtual ~IScreen() = default;

    // Called when screen becomes active
    virtual void onEnter() {}

    // Called when screen becomes inactive
    virtual void onExit() {}

    // Called every frame to render the screen
    virtual void render() = 0;
};