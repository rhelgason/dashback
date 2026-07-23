#pragma once

#include "../core/Algorithm.hpp"

#include <utility>
#include <vector>

namespace dashback {

// Plays back a fixed, previously-solved input sequence from frame 0. Used for
// end-to-end replay of a stored solution — it never learns and never asks for
// another attempt, so if it completes you watched a genuine full run, and if it
// dies the stored solution was unsound (the controller then discards it).
class ReplayAlgorithm : public Algorithm {
public:
    explicit ReplayAlgorithm(std::vector<bool> seq) : m_seq(std::move(seq)) {}

    std::string name() const override { return "replay"; }

    InputState decide(const StepContext& ctx) override {
        int f = ctx.frame;
        return {f >= 0 && f < static_cast<int>(m_seq.size()) && m_seq[f]};
    }

    bool wantsAnotherAttempt() const override { return false; }
    std::vector<bool> solution() const override { return m_seq; }

private:
    std::vector<bool> m_seq;
};

} // namespace dashback
