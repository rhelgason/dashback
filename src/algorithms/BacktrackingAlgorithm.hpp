#pragma once

#include "../core/Algorithm.hpp"

#include <vector>

namespace dashback {

// True depth-first backtracking over jump insertions.
//
// State is a stack of committed "decisions" (each a jump at a chosen frame). To
// get past the current death point we probe candidate jump frames backward from
// the death; the first that extends our furthest progress is pushed and we move
// on. When a branch dead-ends — no candidate in the allowed range extends
// progress — we POP the last decision (undo that jump) and resume its search
// from the next earlier candidate. So a jump that looked good but led nowhere is
// undone and a different one is tried, instead of wedging (the greedy failure).
//
// Search is pruned to jumps that strictly extend the furthest death frame (a
// tractable heuristic for cube obstacles); it explores single-frame taps, which
// fully represent cube play. It is far more robust than greedy but not a blind
// exhaustive search.
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
    // A committed jump plus what's needed to resume searching this slot if the
    // branch above it dead-ends and we pop back here.
    struct Decision {
        int jumpFrame;   // frame this decision inserted a jump at
        int baseDeath;   // furthest death before this jump was added
        int lowerBound;  // earliest frame this slot may probe
        int nextProbe;   // next candidate to try for this slot on backtrack
    };

    void setJump(int frame, bool on);
    void backtrack(); // pop decisions until one has an untried candidate, else exhaust

    std::vector<bool> m_committed;   // current jump set (derived from the stack)
    std::vector<Decision> m_stack;   // committed decisions, in order
    int m_baseDeath = -1;            // death of the current committed set (-1 = not yet known)
    int m_lowerBound = 0;            // earliest frame the current probe may reach
    int m_probe = -1;                // this attempt's trial jump frame (-1 = baseline, no trial)
    int m_granularity = 1;
    bool m_exhausted = false;
};

} // namespace dashback
