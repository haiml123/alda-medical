#pragma once

// Base interface for all screens
class IScreen {
public:
    virtual ~IScreen() = default;

    // Called when screen becomes active
    virtual void on_enter() {}

    // Called when screen becomes inactive
    virtual void on_exit() {}

    // Called every frame to update logic
    virtual void update(float delta_time) {}

    // Called every frame to render the screen
    virtual void render() = 0;
};
