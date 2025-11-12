#pragma once

#include "../core/app_state_manager.h"

namespace elda::models {

    /**
     * Abstract base class for all MVP models
     * Provides common state observation functionality
     */
    class MVPBaseModel {
    public:
        explicit MVPBaseModel(AppStateManager& stateManager)
            : stateManager_(stateManager) {}

        virtual ~MVPBaseModel() = default;

        // Observer pattern - available to all models
        AppStateManager::ObserverHandle addStateObserver(
            AppStateManager::StateObserver observer
        ) {
            return stateManager_.AddObserver(observer);
        }

        void removeStateObserver(AppStateManager::ObserverHandle handle) {
            stateManager_.RemoveObserver(handle);
        }

    protected:
        // Subclasses can access state manager for state changes
        AppStateManager& stateManager_;

        // Convenience method for getting current state
        const AppState& getState() const {
            return stateManager_.GetState();
        }
    };

} // namespace elda