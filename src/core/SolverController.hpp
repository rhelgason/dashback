#pragma once

#include "Algorithm.hpp"
#include "MetricsRecorder.hpp"
#include "Types.hpp"

#include <chrono>
#include <memory>
#include <string>

class PlayLayer;
class GJBaseGameLayer;

namespace dashback {

// The orchestrator. A single instance owns the currently-selected algorithm and
// the metrics recorder, and it is driven by the game hooks:
//
//   PlayLayer::init          -> onLevelStart   (pick algorithm, start attempt 1)
//   GJBaseGameLayer::         -> onStep         (deterministic frame + input)
//     processCommands
//   PlayLayer::destroyPlayer -> onDeath         (record, learn, schedule reset)
//   PlayLayer::resetLevel    -> onAttemptStart  (next attempt)
//   PlayLayer::levelComplete -> onComplete      (record success, stop)
//   PlayLayer::onExit        -> onLevelEnd      (tear down)
//
// The key correctness idea: `onStep` runs from processCommands, which the game
// calls exactly once per fixed physics step (240/s), NOT once per rendered
// frame. Counting those calls gives a frame index that is identical across
// attempts regardless of framerate — the root fix for the old replay flakiness.
class SolverController {
public:
    static SolverController& get();

    void onLevelStart(PlayLayer* playLayer);
    void onAttemptStart();
    void onStep(GJBaseGameLayer* gameLayer);
    void onDeath(PlayLayer* playLayer);
    void onComplete(PlayLayer* playLayer);
    void onLevelEnd();

    bool active() const { return m_active; }

    // Text for the on-screen HUD, refreshed by PlayLayer::postUpdate.
    std::string hudText() const;

private:
    SolverController() = default;

    void beginAttempt();
    bool reachedAttemptLimit() const;

    std::unique_ptr<Algorithm> m_algo;
    MetricsRecorder m_metrics;
    LevelInfo m_level;

    PlayLayer* m_playLayer = nullptr;

    bool m_active = false;          // solver enabled and running for this level
    bool m_awaitingFirstStep = false; // debounces duplicate resetLevel() calls
    bool m_deadThisAttempt = false;   // dedupes p1/p2 destroyPlayer calls
    bool m_solved = false;

    int m_attempt = 0;             // 1-based attempt counter
    int m_frame = 0;               // deterministic physics-step index
    int m_maxAttempts = 0;         // 0 = unlimited
    float m_bestProgress = 0.f;    // best progress this attempt (0..1)
    bool m_lastHold = false;       // last applied jump state (edge detection)

    long long m_sessionId = 0;

    std::chrono::steady_clock::time_point m_attemptStart;
};

} // namespace dashback
