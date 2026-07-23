#pragma once

#include "../core/Algorithm.hpp"

#include <vector>

namespace dashback {

// Greedy jump-insertion search (a practical take on backtracking).
//
// The prototype flipped the latest non-jump frame and discarded everything after
// it, so it only ever brute-forced the jump pattern right before the death point
// and never kept progress. This version instead *commits* progress:
//
//   * Play the committed jump sequence plus one trial jump at `m_probe`.
//   * If that trial pushes the death point further than ever before, commit the
//     jump and start probing just before the NEW death point.
//   * Otherwise move the probe one frame earlier and try again.
//
// So each obstacle is cleared by finding a single well-timed jump, locking it in,
// and moving on — which makes visible, monotonic progress. It's still naive (one
// jump at a time, greedy, no un-committing a bad jump), but it actually advances.
class BacktrackingAlgorithm : public Algorithm {
public:
    std::string name() const override { return "backtracking"; }

    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    bool wantsAnotherAttempt() const override { return !m_exhausted; }

private:
    std::vector<bool> m_committed; // locked-in jumps, indexed by frame
    int m_bestDeath = 0;           // furthest frame reached with committed jumps
    int m_probe = -1;              // frame we're trying an extra jump at (-1 = not yet started)
    bool m_exhausted = false;
};

} // namespace dashback
