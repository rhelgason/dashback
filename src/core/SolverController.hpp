#pragma once

#include "Algorithm.hpp"
#include "MetricsRecorder.hpp"
#include "Types.hpp"

#include <chrono>
#include <memory>
#include <string>

class PlayLayer;
class GJBaseGameLayer;
class CheckpointObject;

namespace dashback {

// The orchestrator. Owns the current algorithm + metrics and is driven by the
// game hooks. It runs in one of two modes:
//
//   Search  — run the selected algorithm to find an input sequence. Optionally
//             uses GD checkpoints to restart near the frontier instead of from
//             frame 0 (fast-restart). On completion the full sequence is saved.
//   Replay  — a level with a saved solution plays that sequence from frame 0 at
//             watchable speed: a genuine end-to-end run. If it fails to complete,
//             the stored solution is discarded and the controller re-searches.
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
    bool inLevel() const { return m_playLayer != nullptr; }

    // Live game-speed control (bound to keys). delta is added to the multiplier.
    void adjustSpeed(float delta);

    std::string hudText() const;

private:
    enum class Mode { Search, Replay };

    SolverController() = default;

    void beginAttempt(int startFrame);
    void scheduleRestart();       // search mode: checkpoint restore or full reset
    void startSearch();           // (re)initialize a from-scratch search
    void applySpeed();
    bool reachedAttemptLimit() const;

    std::unique_ptr<Algorithm> m_algo;
    MetricsRecorder m_metrics;
    LevelInfo m_level;
    std::string m_algoId;

    PlayLayer* m_playLayer = nullptr;

    Mode m_mode = Mode::Search;
    bool m_active = false;
    bool m_awaitingFirstStep = false;
    bool m_awaitingCheckpointStart = false; // ignore the resetLevel hook during a checkpoint restore
    bool m_deadThisAttempt = false;
    bool m_solved = false;
    bool m_fastRestart = false;

    int m_attempt = 0;
    int m_frame = 0;
    int m_maxAttempts = 0;
    float m_bestProgress = 0.f;
    float m_bestEver = 0.f;
    bool m_lastHold = false;

    // Checkpoint state (search + fast-restart).
    CheckpointObject* m_checkpoint = nullptr;
    int m_checkpointFrame = -1;

    float m_speed = 1.f;
    long long m_sessionId = 0;

    std::chrono::steady_clock::time_point m_attemptStart;
};

} // namespace dashback
