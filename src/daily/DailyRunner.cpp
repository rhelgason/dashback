#include "DailyRunner.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace dashback {

DailyRunner& DailyRunner::get() {
    static DailyRunner instance;
    return instance;
}

bool DailyRunner::enabled() const {
    return Mod::get()->getSettingValue<bool>("auto-solve-daily");
}

void DailyRunner::onMenuLayer(MenuLayer* /*menuLayer*/) {
    if (!enabled() || m_launched) return;
    m_launched = true;

    log::warn("dashback: auto-solve-daily is ON, but daily auto-navigation is "
              "not yet implemented — open a level manually for now");

    // TODO(daily): fetch the current Daily level, open it, and let the PlayLayer
    // hooks + SolverController take over. Rough shape (needs a live client to
    // validate the networking + UI, so it is deliberately left as a follow-up
    // rather than untested guesswork):
    //
    //   auto glm = GameLevelManager::sharedState();
    //   glm->getGJDailyLevelState(GJTimedLevelType::Daily); // current id/timer
    //   // download the level (glm->getOnlineLevels / DailyLevelPage), then on
    //   // completion:
    //   auto scene = PlayLayer::scene(level, false, false);
    //   CCDirector::sharedDirector()->pushScene(
    //       CCTransitionFade::create(0.5f, scene));
    //
    // Once the mod is proven end-to-end and a best algorithm is chosen, this
    // becomes a self-contained task: drive to the daily, then loop the solver
    // until it reports SOLVED via the metrics/hud state.
}

} // namespace dashback
