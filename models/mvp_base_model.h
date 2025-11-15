#pragma once

#include "../core/app_state_manager.h"

namespace elda::models {

    class MVPBaseModel {
    public:
        explicit MVPBaseModel(AppStateManager& state_manager)
            : state_manager_(state_manager) {}

        virtual ~MVPBaseModel() = default;

        AppStateManager::ObserverHandle add_state_observer(
            AppStateManager::StateObserver observer
        ) {
            return state_manager_.add_observer(observer);
        }

        void remove_state_observer(AppStateManager::ObserverHandle handle) {
            state_manager_.remove_observer(handle);
        }

    protected:
        AppStateManager& state_manager_;

        const AppState& get_state() const {
            return state_manager_.get_state();
        }
    };

}
