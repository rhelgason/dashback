#pragma once

#include "../core/Algorithm.hpp"

#include <random>
#include <vector>

namespace dashback {

// A pure random baseline: each attempt taps jump independently at random with a
// fixed probability per frame. It does not learn, which is exactly the point —
// it establishes the "floor" that smarter algorithms should beat when you look
// at the metrics CSV. Seeded per-attempt so a given attempt is reproducible.
class RandomSearchAlgorithm : public Algorithm {
public:
    std::string name() const override { return "random"; }

    void onAttemptStart(int attempt) override;
    InputState decide(const StepContext& ctx) override;

private:
    std::mt19937 m_rng{0};
    std::vector<bool> m_holds; // materialized lazily as frames are requested
    double m_jumpProbability = 0.08; // per-frame probability of a tap
};

} // namespace dashback
