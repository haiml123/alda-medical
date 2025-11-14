#include "impedance_viewer_screen.h"

namespace elda::views::impedance_viewer {

    ImpedanceViewerScreen::ImpedanceViewerScreen(AppState& state, AppStateManager& stateManager, AppRouter& router)
        : model_(state.availableChannels ? *state.availableChannels : std::vector<elda::models::Channel>{}, stateManager)
        , view_()
        , presenter_(model_, view_, router)
    {
    }

    void ImpedanceViewerScreen::onEnter() {
        presenter_.OnEnter();
    }

    void ImpedanceViewerScreen::onExit() {
        presenter_.OnExit();
    }

    void ImpedanceViewerScreen::update(float dt) {
        presenter_.Update(dt);
    }

    void ImpedanceViewerScreen::render() {
        presenter_.Render();
    }

} // namespace elda::impedance_viewer