#pragma once

#include "../core/Algorithm.hpp"

#include <vector>

namespace dashback {

// Depth-first backtracking over variable-length HOLD intervals.
//
// The unit of search is a held input [start, start+len) rather than a one-frame
// tap. This is what lets it express flight (ship) input — sustained thrust —
// and consecutive/held jumps, not just isolated taps, while staying pure
// backtracking: it systematically enumerates hold intervals, commits one when it
// extends the furthest death (branch-and-bound pruning), and on a dead-end pops
// the last hold and resumes that slot's enumeration. No game-state heuristics,
// no mode-awareness — the hold search handles cube and ship uniformly.
//
// Enumeration order is length-major (shortest holds first), so cube taps
// (length 1) are still tried first and only longer holds are explored when short
// ones fail. `search-granularity` steps both the start and the length;
// `max-hold-length` bounds a single interval (long thrusts chain adjacent ones).
class BacktrackingAlgorithm : public Algorithm {
public:
    std::string name() const override { return "backtracking"; }

    void onLevelStart(const LevelInfo& level) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    void onComplete(const AttemptResult& result) override;
    bool wantsAnotherAttempt() const override { return !m_exhausted; }

    std::vector<bool> solution() const override { return m_committed; }

private:
    struct Decision {
        int start, len;    // committed hold interval [start, start+len)
        int baseDeath;     // furthest death before this hold was added
        int lowerBound;    // earliest start this slot may probe
        int searchTop;     // highest start this slot may probe (baseDeath-1)
        int resumeStart;   // enumeration state to resume at on backtrack
        int resumeLen;
    };

    void setHold(int start, int len, bool on);
    bool advanceProbe();  // step to the next (start,len) in enumeration order
    void backtrack();     // pop decisions until one has an untried candidate

    std::vector<bool> m_committed;  // per-frame hold state (derived from the stack)
    std::vector<Decision> m_stack;

    int m_baseDeath = -1;  // death of the current committed set (-1 = not yet known)
    int m_lowerBound = 0;  // earliest start for the current slot
    int m_searchTop = -1;  // highest start for the current slot
    int m_probeStart = -1; // this attempt's trial hold start (-1 = baseline, no trial)
    int m_probeLen = 0;    // this attempt's trial hold length

    int m_granularity = 1;
    int m_maxHold = 60;
    bool m_exhausted = false;
};

} // namespace dashback
