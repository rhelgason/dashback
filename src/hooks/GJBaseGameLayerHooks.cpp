#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "../core/SolverController.hpp"

using namespace geode::prelude;

// processCommands is the game's per-physics-step function (called once per fixed
// physics step, regardless of framerate). Hooking it — rather than the per-render
// postUpdate the prototype used — is what makes frame counting and input replay
// deterministic. We decide + apply input before the original runs the step.
//
// GD 2.2081 split the step into half-ticks: the signature gained isHalfTick and
// isLastTick. We still count every invocation as a step — that sequence is
// deterministic for identical input, which is all replay requires. (If half-tick
// granularity ever hurts solves, gate onStep on !isHalfTick or isLastTick here.)
class $modify(DBGameLayer, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        dashback::SolverController::get().onStep(this);
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};
