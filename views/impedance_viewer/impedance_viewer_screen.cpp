#include "impedance_viewer_screen.h"

namespace elda::impedance_viewer {

    ImpedanceViewerScreen::ImpedanceViewerScreen(AppState& state, AppStateManager& stateManager) {
        // AppState::availableChannels is a POINTER in your codebase.
        // Model wants a REFERENCE. Safely handle nullptr with a static empty vec.
        static const std::vector<elda::models::Channel> kEmptyChannels;
        const std::vector<elda::models::Channel>& channels =
            (state.availableChannels ? *state.availableChannels : kEmptyChannels);

        auto model = std::make_unique<ImpedanceViewerModel>(channels, stateManager);
        auto view  = std::make_unique<ImpedanceViewerView>();
        presenter_ = std::make_unique<ImpedanceViewerPresenter>(std::move(model), std::move(view));
    }

    void ImpedanceViewerScreen::onEnter()        { presenter_->OnEnter(); }
    void ImpedanceViewerScreen::onExit()         { presenter_->OnExit(); }
    void ImpedanceViewerScreen::update(float dt) { presenter_->Update(dt); }
    void ImpedanceViewerScreen::render() {
        if (!isOpen_ || !presenter_) return;
        presenter_->Render(&isOpen_);
    }

} // namespace elda::impedance_viewer
