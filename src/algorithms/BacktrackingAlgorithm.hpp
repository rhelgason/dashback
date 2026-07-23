#pragma once

#include "../core/Algorithm.hpp"

#include <vector>

namespace dashback {

// Greedy jump-insertion search (a practical take on backtracking).
//
//   * Play the committed jump sequence plus one trial jump at `m_probe`.
//   * If that trial pushes the death point further than ever before, commit the
//     jump and start probing just before the NEW death point.
//   * Otherwise move the probe earlier and try again.
//
// The probe never walks earlier than the last committed jump, so everything up to
// that frame is final — which lets the controller checkpoint there and only
// replay the short segment to the next obstacle.
class BacktrackingAlgorithm : public Algorithm {
public:
    std::string name() const override { return "backtracking"; }

    void onLevelStart(const LevelInfo& level) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    bool wantsAnotherAttempt() const override { return !m_exhausted; }

    std::vector<bool> solution() const override { return m_committed; }
    int frontierFrame() const override { return m_lastCommitted > 0 ? m_lastCommitted : 0; }
    int nextChangeFrame() const override { return m_probe > 0 ? m_probe : 0; }

private:
    std::vector<bool> m_committed; // locked-in jumps, indexed by frame
    int m_bestDeath = 0;           // furthest frame reached with committed jumps
    int m_probe = -1;              // frame we're trying an extra jump at (-1 = not yet started)
    int m_lastCommitted = 0;       // frame of the most recent committed jump (search frontier)
    int m_granularity = 1;         // frames the probe steps per attempt (search-granularity setting)
    bool m_exhausted = false;
};

} // namespace dashback
