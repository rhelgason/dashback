#pragma once

#include "../core/Algorithm.hpp"

#include <vector>

namespace dashback {

// A deterministic re-implementation of the original prototype. It maintains a
// per-frame plan of jump holds; on each death it flips the latest "no-hold"
// frame before the death point to a hold and discards everything after,
// systematically pushing a jump earlier through the sequence.
//
// This is a naive brute-force search kept as a baseline — it only really works
// on trivial levels — but it is now frame-deterministic, so it no longer suffers
// the replay desync of the original.
class BacktrackingAlgorithm : public Algorithm {
public:
    std::string name() const override { return "backtracking"; }

    void onAttemptStart(int attempt) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    bool wantsAnotherAttempt() const override { return !m_exhausted; }

private:
    std::vector<bool> m_holds; // planned jump-hold state per frame
    bool m_exhausted = false;
};

} // namespace dashback
