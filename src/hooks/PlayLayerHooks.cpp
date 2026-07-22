#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "../core/SolverController.hpp"

using namespace geode::prelude;

// All PlayLayer-side lifecycle wiring. The HUD label lives in per-instance
// `m_fields` (not a global static like the prototype), so it is destroyed with
// its PlayLayer and can never dangle across level re-entry.
class $modify(DBPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* hudLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        dashback::SolverController::get().onLevelStart(this);

        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setAnchorPoint({0.f, 1.f});
        label->setScale(0.5f);
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        label->setPosition({10.f, winSize.height - 10.f});
        this->addChild(label, 1000);
        m_fields->hudLabel = label;

        return true;
    }

    // Render-frame cadence is fine for a HUD; determinism only matters for input.
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (auto label = m_fields->hudLabel) {
            label->setString(dashback::SolverController::get().hudText().c_str());
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        dashback::SolverController::get().onDeath(this);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        dashback::SolverController::get().onAttemptStart();
    }

    void levelComplete() {
        dashback::SolverController::get().onComplete(this);
        PlayLayer::levelComplete();
    }

    void onExit() {
        dashback::SolverController::get().onLevelEnd();
        PlayLayer::onExit();
    }
};
