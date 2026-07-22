#include "RandomSearchAlgorithm.hpp"

namespace dashback {

void RandomSearchAlgorithm::onAttemptStart(int attempt) {
    // Seed per-attempt so each attempt is independent but reproducible.
    m_rng.seed(static_cast<std::mt19937::result_type>(attempt));
    m_holds.clear();
}

InputState RandomSearchAlgorithm::decide(const StepContext& ctx) {
    int f = ctx.frame;
    std::bernoulli_distribution tap(m_jumpProbability);
    while (static_cast<int>(m_holds.size()) <= f) {
        m_holds.push_back(tap(m_rng));
    }
    return {m_holds[static_cast<std::size_t>(f)]};
}

} // namespace dashback
