#pragma once

#include "../core/Algorithm.hpp"
#include "SequenceSearch.hpp"

#include <random>
#include <vector>

namespace dashback {

// (1+1) local search over hold-sequences: keep a "state" sequence, each attempt
// play a death-focused mutation of it, and accept per the rule below. Two flavors
// share this one class:
//   hillclimb — greedy: accept only if the mutation reached at least as far.
//   annealing — also accept *worse* mutations with probability exp(dFit / T),
//               with T cooling over attempts, to escape local optima.
// `m_best` always tracks the furthest sequence ever seen (the solution); the SA
// "state" may wander to worse sequences to explore.
class HillClimbingAlgorithm : public Algorithm {
public:
    explicit HillClimbingAlgorithm(bool annealing) : m_annealing(annealing) {}

    std::string name() const override { return m_annealing ? "annealing" : "hillclimb"; }

    void onLevelStart(const LevelInfo& level) override;
    void onAttemptStart(int attempt) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    void onComplete(const AttemptResult& result) override;

    std::vector<bool> solution() const override { return m_best; }

private:
    bool m_annealing;
    std::mt19937 m_rng{2024};

    seq::Genome m_best;    // furthest sequence ever (the answer)
    int m_bestFit = -1;

    seq::Genome m_state;   // current search position (SA may wander below best)
    int m_stateFit = -1;
    int m_stateDeath = 0;  // where the state dies (mutation center)

    seq::Genome m_current; // this attempt's candidate

    double m_mutationRate = 0.08;
    int m_mutationWindow = 80;
    double m_holdProb = 0.08;
    double m_temp = 0.0;
    double m_cooling = 0.999;
};

} // namespace dashback
