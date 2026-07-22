#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "../core/SolverController.hpp"

using namespace geode::prelude;

// processCommands is the game's per-physics-step function (called once per fixed
// 1/240s step, regardless of framerate). Hooking it — rather than the per-render
// postUpdate the prototype used — is what makes frame counting and input replay
// deterministic. We decide + apply input before the original runs the step.
class $modify(DBGameLayer, GJBaseGameLayer) {
    void processCommands(float dt) {
        dashback::SolverController::get().onStep(this);
        GJBaseGameLayer::processCommands(dt);
    }
};
