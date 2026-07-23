#pragma once

#include <string>

// Forward declarations of the Geometry Dash classes we hand to algorithms as an
// "escape hatch". Keeping these out of the public headers means algorithm code
// only needs the heavy Geode includes if it actually reaches for live state.
class PlayerObject;
class GJBaseGameLayer;

namespace dashback {

// The player's current gameplay mode. The per-frame hold bit means the same to
// every algorithm, but GD interprets it differently per mode, so exposing the
// mode lets algorithms bias their search:
//   Cube   — tap to jump (edge-triggered, from ground); hold = auto-jump on land
//   Ship   — hold = thrust up, release = fall (continuous)
//   Ball   — tap to flip gravity (edge-triggered)
//   UFO    — tap for an upward impulse each press (edge-triggered, mid-air)
//   Wave   — hold = up 45 deg, release = down 45 deg (continuous, twitchy)
//   Robot  — hold length sets jump height (from ground)
//   Spider — tap to teleport to the opposite surface (edge-triggered)
//   Swing  — tap to flip thrust direction (edge-triggered, continuous)
enum class GameMode { Cube, Ship, Ball, UFO, Wave, Robot, Spider, Swing, Unknown };

// The decision an algorithm makes for a single physics step. Classic modes only
// need the jump button; the same bit drives every mode (see GameMode).
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
    GameMode mode = GameMode::Cube;

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
