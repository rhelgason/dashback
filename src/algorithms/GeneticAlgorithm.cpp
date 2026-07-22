#include "GeneticAlgorithm.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <numeric>

namespace dashback {

void GeneticAlgorithm::onLevelStart(const LevelInfo& /*level*/) {
    m_rng.seed(1337);
    m_population.assign(kPopulationSize, Genome{});
    m_fitness.assign(kPopulationSize, 0.f);
    m_current = 0;
    m_generation = 0;
}

void GeneticAlgorithm::onAttemptStart(int /*attempt*/) {
    m_bit = 0;
}

InputState GeneticAlgorithm::decide(const StepContext& /*ctx*/) {
    Genome& g = m_population[static_cast<std::size_t>(m_current)];
    // Genomes grow lazily as an attempt progresses further than any before it;
    // the grown bits become heritable material for crossover.
    if (m_bit >= g.size()) {
        std::bernoulli_distribution tap(0.08);
        g.push_back(tap(m_rng));
    }
    bool hold = g[m_bit];
    ++m_bit;
    return {hold};
}

void GeneticAlgorithm::onDeath(const DeathInfo& info) {
    recordFitness(info.progress);
}

void GeneticAlgorithm::onComplete(const AttemptResult& /*result*/) {
    recordFitness(1.0f);
}

void GeneticAlgorithm::recordFitness(float progress) {
    if (m_current < 0 || m_current >= kPopulationSize) return;
    m_fitness[static_cast<std::size_t>(m_current)] = progress;
    ++m_current;
    evolveIfGenerationComplete();
}

void GeneticAlgorithm::evolveIfGenerationComplete() {
    if (m_current < kPopulationSize) return;

    // Rank genomes by fitness (best first).
    std::vector<int> order(kPopulationSize);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(),
        [&](int a, int b) { return m_fitness[a] > m_fitness[b]; });

    float bestFitness = m_fitness[order.front()];

    std::vector<Genome> next;
    next.reserve(kPopulationSize);

    // Elitism: carry the best genomes forward unchanged.
    for (int e = 0; e < kEliteCount; ++e) {
        next.push_back(m_population[static_cast<std::size_t>(order[e])]);
    }

    auto tournament = [&]() -> const Genome& {
        std::uniform_int_distribution<int> pick(0, kPopulationSize - 1);
        int a = pick(m_rng), b = pick(m_rng);
        return (m_fitness[a] >= m_fitness[b])
            ? m_population[static_cast<std::size_t>(a)]
            : m_population[static_cast<std::size_t>(b)];
    };

    std::bernoulli_distribution mutate(kMutationRate);
    std::bernoulli_distribution coin(0.5);

    while (static_cast<int>(next.size()) < kPopulationSize) {
        const Genome& p1 = tournament();
        const Genome& p2 = tournament();
        std::size_t len = std::max(p1.size(), p2.size());
        std::size_t cut = (len > 0)
            ? std::uniform_int_distribution<std::size_t>(0, len)(m_rng)
            : 0;

        Genome child;
        child.reserve(len);
        for (std::size_t i = 0; i < len; ++i) {
            const Genome& src = (i < cut) ? p1 : p2;
            bool bit = (i < src.size()) ? src[i] : coin(m_rng);
            if (mutate(m_rng)) bit = !bit;
            child.push_back(bit);
        }
        next.push_back(std::move(child));
    }

    m_population = std::move(next);
    m_fitness.assign(kPopulationSize, 0.f);
    m_current = 0;
    ++m_generation;

    geode::log::info("dashback[genetic]: generation {} done, best fitness {:.1f}%",
        m_generation, bestFitness * 100.f);
}

} // namespace dashback
