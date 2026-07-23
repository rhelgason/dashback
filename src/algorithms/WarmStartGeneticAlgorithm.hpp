#pragma once

#include "../core/Algorithm.hpp"
#include "GeneticAlgorithm.hpp"
#include "PolicyAlgorithm.hpp"
#include "SequenceSearch.hpp"

#include <string>
#include <vector>

namespace dashback {

// ML-as-a-prior: run the trained policy once to get a full input sequence, then
// hand that sequence to the genetic search as its initial population instead of
// starting from random. If the policy is decent, the search begins most of the
// way to a solution and refines the rest in far fewer attempts. The genetic side
// remains the source of truth (it verifies/repairs), so a bad policy only costs
// one seeding run — it can't produce a wrong "solution".
//
// If no model is loaded, this degrades gracefully to a plain genetic search.
class WarmStartGeneticAlgorithm : public Algorithm {
public:
    std::string name() const override { return "genetic-warm"; }

    void onLevelStart(const LevelInfo& level) override;
    void onAttemptStart(int attempt) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    void onComplete(const AttemptResult& result) override;
    bool wantsAnotherAttempt() const override;
    std::vector<bool> solution() const override;

private:
    PolicyAlgorithm m_policy;
    GeneticAlgorithm m_ga;
    bool m_seeding = false;       // in the policy seed run
    bool m_seedSolved = false;    // policy solved it during the seed run
    seq::Genome m_seedActions;    // actions recorded during the seed run
};

} // namespace dashback
