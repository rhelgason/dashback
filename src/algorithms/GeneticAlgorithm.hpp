#pragma once

#include "../core/Algorithm.hpp"

#include <random>
#include <vector>

namespace dashback {

// An evolutionary solver. Each genome is a per-frame bitstring of jump holds.
// One genome is evaluated per attempt (fitness = best progress reached); once
// the whole population has been played, the population is evolved (elitism +
// tournament selection + single-point crossover + bit-flip mutation) and the
// next generation is played. This is the strongest of the built-ins and the
// clearest demonstration of state carried across attempts and generations.
class GeneticAlgorithm : public Algorithm {
public:
    std::string name() const override { return "genetic"; }

    void onLevelStart(const LevelInfo& level) override;
    void onAttemptStart(int attempt) override;
    InputState decide(const StepContext& ctx) override;
    void onDeath(const DeathInfo& info) override;
    void onComplete(const AttemptResult& result) override;

private:
    using Genome = std::vector<bool>;

    void recordFitness(float progress);
    void evolveIfGenerationComplete();

    static constexpr int kPopulationSize = 24;
    static constexpr int kEliteCount = 4;
    static constexpr double kMutationRate = 0.02;

    std::mt19937 m_rng{1337};
    std::vector<Genome> m_population;
    std::vector<float> m_fitness;
    int m_current = 0;      // index of the genome being evaluated this attempt
    int m_generation = 0;
    std::size_t m_bit = 0;  // read cursor into the current genome during play
};

} // namespace dashback
