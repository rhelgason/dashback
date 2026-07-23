#include "WarmStartGeneticAlgorithm.hpp"

#include <Geode/Geode.hpp>

namespace dashback {

void WarmStartGeneticAlgorithm::onLevelStart(const LevelInfo& level) {
    m_policy.onLevelStart(level);
    m_ga.onLevelStart(level);
    // Only bother with a policy seed run if a model is actually loaded.
    m_seeding = m_policy.ready();
    m_seedSolved = false;
    m_seedActions.clear();
    if (!m_seeding) {
        geode::log::info("dashback[genetic-warm]: no policy model — running plain genetic");
    }
}

void WarmStartGeneticAlgorithm::onAttemptStart(int attempt) {
    if (m_seeding) m_policy.onAttemptStart(attempt);
    else m_ga.onAttemptStart(attempt);
}

InputState WarmStartGeneticAlgorithm::decide(const StepContext& ctx) {
    if (m_seeding) {
        InputState in = m_policy.decide(ctx);
        m_seedActions.push_back(in.hold);
        return in;
    }
    return m_ga.decide(ctx);
}

void WarmStartGeneticAlgorithm::onDeath(const DeathInfo& info) {
    if (m_seeding) {
        // Seed run finished — use its recorded actions as the GA's starting point.
        m_policy.onDeath(info);
        m_ga.seedWith(m_seedActions);
        m_seeding = false;
        return;
    }
    m_ga.onDeath(info);
}

void WarmStartGeneticAlgorithm::onComplete(const AttemptResult& result) {
    if (m_seeding) {
        m_seedSolved = true; // the policy solved it outright during seeding
        return;
    }
    m_ga.onComplete(result);
}

bool WarmStartGeneticAlgorithm::wantsAnotherAttempt() const {
    // During seeding we always continue (into the GA phase after the seed run).
    if (m_seeding) return true;
    return m_ga.wantsAnotherAttempt();
}

std::vector<bool> WarmStartGeneticAlgorithm::solution() const {
    if (m_seedSolved) return m_seedActions;
    return m_ga.solution();
}

} // namespace dashback
