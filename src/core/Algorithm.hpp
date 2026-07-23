#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

namespace dashback {

// The one interface every solver implements. The lifecycle is:
//
//   onLevelStart(level)          once, when a level is opened
//     onAttemptStart(1)          before attempt 1 begins
//       decide(ctx) * N          once per physics step while playing
//     onDeath(info) | onComplete(result)
//     onAttemptStart(2) ...      repeated until wantsAnotherAttempt() is false
//
// `decide` covers both families of algorithm cleanly:
//   * replay-based (backtracking, genetic): return a pre-planned input for the
//     frame, and mutate the plan in onDeath / onComplete.
//   * reactive/online (rule-based, RL): inspect ctx and decide live.
//
// To add an algorithm: subclass this, implement name()/decide(), register it in
// algorithms/Builtins.cpp, and it becomes selectable via the "algorithm"
// setting. Nothing else in the codebase needs to change.
class Algorithm {
public:
    virtual ~Algorithm() = default;

    // Stable identifier, also used as the value of the "algorithm" setting and
    // in the metrics CSV. Keep it short and kebab-case.
    virtual std::string name() const = 0;

    // Called once when a level is opened, before the first attempt.
    virtual void onLevelStart(const LevelInfo&) {}

    // Called before each attempt (including the first). `attempt` is 1-based.
    virtual void onAttemptStart(int /*attempt*/) {}

    // The core of the algorithm: decide the input for this physics step.
    virtual InputState decide(const StepContext& ctx) = 0;

    // Called once when the player dies. Learners react here.
    virtual void onDeath(const DeathInfo&) {}

    // Called once when the level is completed.
    virtual void onComplete(const AttemptResult&) {}

    // Return false to tell the controller to stop retrying (search exhausted,
    // solved, converged, etc.). Defaults to "keep going".
    virtual bool wantsAnotherAttempt() const { return true; }

    // The full per-frame input sequence the algorithm currently believes in.
    // Persisted verbatim when a level is completed so it can be replayed E2E.
    virtual std::vector<bool> solution() const { return {}; }

    // Checkpoint support (search acceleration). `frontierFrame` is the frame up
    // to which the input sequence is final and stable — safe to snapshot state
    // at. `nextChangeFrame` is the earliest frame the *next* attempt might differ
    // from that stable prefix; if it is >= the checkpoint frame, the controller
    // can restart from the checkpoint instead of from frame 0. Algorithms that
    // rewrite their whole sequence each attempt leave these at 0 (no benefit).
    virtual int frontierFrame() const { return 0; }
    virtual int nextChangeFrame() const { return 0; }
};

} // namespace dashback
