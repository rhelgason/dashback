#pragma once

#include "Types.hpp"

#include <string>

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
};

} // namespace dashback
