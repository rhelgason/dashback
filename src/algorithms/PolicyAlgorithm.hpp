#pragma once

#include "../core/Algorithm.hpp"

#include <string>
#include <vector>

namespace dashback {

// Runs a trained neural-network policy reactively: each frame it senses the
// world (Observation), feeds it through a small MLP, and holds if the output
// exceeds 0.5. This is the consumer side of the ML pipeline — it loads weights
// produced offline by ml/train.py. With no weights present it is inert (never
// holds) and logs a hint, so the mod still runs fine.
//
// It's deterministic, so it plays a level once; if it completes, that run is
// saved as the solution. (Future: use this policy to warm-start the genetic
// search rather than run standalone.)
class PolicyAlgorithm : public Algorithm {
public:
    std::string name() const override { return "policy"; }

    void onLevelStart(const LevelInfo& level) override;
    void onAttemptStart(int attempt) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    bool wantsAnotherAttempt() const override { return !m_done; }
    std::vector<bool> solution() const override { return m_played; }

private:
    bool loadModel();
    float forward(const std::vector<float>& x) const;

    // Two-hidden-layer MLP: F -> H (relu) -> H (relu) -> 1 (sigmoid).
    bool m_loaded = false;
    int m_inDim = 0;
    int m_hidden = 0;
    std::vector<float> m_w1, m_b1, m_w2, m_b2, m_w3; // m_b3 is a scalar
    float m_b3 = 0.f;

    std::vector<bool> m_played; // the actions this run took (for saving a solution)
    bool m_done = false;
};

} // namespace dashback
