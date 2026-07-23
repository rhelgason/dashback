#include "HillClimbingAlgorithm.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>

namespace dashback {

void HillClimbingAlgorithm::onLevelStart(const LevelInfo& /*level*/) {
    m_mutationRate = geode::Mod::get()->getSettingValue<double>("mutation-rate");
    m_mutationWindow = std::max(1, static_cast<int>(
        geode::Mod::get()->getSettingValue<int64_t>("mutation-window")));
    m_temp = m_annealing
        ? std::max(0.0, geode::Mod::get()->getSettingValue<double>("annealing-temperature"))
        : 0.0;
    m_cooling = 0.999;

    m_rng.seed(2024);
    m_best.clear();
    m_bestFit = -1;
    m_state.clear();
    m_stateFit = -1;
    m_stateDeath = 0;
    m_current.clear();
}

void HillClimbingAlgorithm::onAttemptStart(int /*attempt*/) {
    if (m_stateFit < 0) {
        m_current.clear(); // first attempt: a fresh random sequence (grown in decide)
    } else {
        m_current = m_state;
        seq::mutateAround(m_current, m_stateDeath, m_mutationWindow,
            m_mutationRate, m_holdProb, m_rng);
    }
}

InputState HillClimbingAlgorithm::decide(const StepContext& ctx) {
    int f = ctx.frame;
    if (f >= static_cast<int>(m_current.size())) {
        std::bernoulli_distribution hold(m_holdProb);
        while (static_cast<int>(m_current.size()) <= f) m_current.push_back(hold(m_rng));
    }
    return {m_current[static_cast<std::size_t>(f)]};
}

void HillClimbingAlgorithm::onDeath(const DeathInfo& info) {
    int fit = info.frame;

    if (fit > m_bestFit) {
        m_bestFit = fit;
        m_best = m_current;
    }

    bool accept = fit >= m_stateFit;
    if (!accept && m_annealing && m_temp > 0.0) {
        double p = std::exp((fit - m_stateFit) / m_temp);
        accept = std::bernoulli_distribution(std::clamp(p, 0.0, 1.0))(m_rng);
    }
    if (accept) {
        m_state = m_current;
        m_stateFit = fit;
        m_stateDeath = info.frame;
    }

    if (m_annealing && m_temp > 0.0) m_temp *= m_cooling;
}

void HillClimbingAlgorithm::onComplete(const AttemptResult& /*result*/) {
    m_best = m_current;
}

} // namespace dashback
