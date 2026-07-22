#pragma once

#include <string>

// Forward declarations of the Geometry Dash classes we hand to algorithms as an
// "escape hatch". Keeping these out of the public headers means algorithm code
// only needs the heavy Geode includes if it actually reaches for live state.
class PlayerObject;
class GJBaseGameLayer;

namespace dashback {

// The decision an algorithm makes for a single physics step. Classic mode only
// needs the jump button, so this is intentionally minimal; add fields here (e.g.
// left/right for platformer mode) as algorithms grow to need them.
struct InputState {
    bool hold = false; // whether the jump button should be held this step
};

// Read-only snapshot of the game handed to an algorithm every physics step.
// `frame` is a DETERMINISTIC physics-step index (see SolverController) — it is
// the same across attempts for the same input, which is what makes replay-based
// algorithms reliable. Prefer the pre-computed fields; the raw pointers are an
// escape hatch for algorithms that need state we haven't surfaced yet.
struct StepContext {
    int frame = 0;
    float progress = 0.f; // 0..1 fraction of the level completed
    float playerX = 0.f;
    float playerY = 0.f;
    double playerYVelocity = 0.0;
    bool onGround = false;
    bool upsideDown = false;
    bool isShip = false;

    PlayerObject* player = nullptr;
    GJBaseGameLayer* gameLayer = nullptr;
};

// Passed to Algorithm::onDeath so a learner can react to where/how it failed.
struct DeathInfo {
    int frame = 0;        // physics-step index the death occurred on
    float progress = 0.f; // best progress reached this attempt (0..1)
};

// Static description of the level being solved.
struct LevelInfo {
    int levelID = 0;
    std::string name;
    float length = 0.f;
};

// One row of the metrics log: the outcome of a single attempt.
struct AttemptResult {
    int attempt = 0;
    bool success = false;
    float bestProgress = 0.f; // 0..1
    int deathFrame = 0;       // physics-step index at death (== frames on success)
    int frames = 0;          // total physics steps this attempt ran
    double wallMs = 0.0;     // wall-clock duration of the attempt
};

} // namespace dashback
