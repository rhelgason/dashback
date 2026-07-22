#include "BacktrackingAlgorithm.hpp"

#include <algorithm>

namespace dashback {

void BacktrackingAlgorithm::onAttemptStart(int /*attempt*/) {
    // The plan persists across attempts; onDeath is what mutates it.
}

InputState BacktrackingAlgorithm::decide(const StepContext& ctx) {
    int f = ctx.frame;
    while (static_cast<int>(m_holds.size()) <= f) m_holds.push_back(false);
    return {m_holds[static_cast<std::size_t>(f)]};
}

void BacktrackingAlgorithm::onDeath(const DeathInfo& info) {
    // Flip the latest not-yet-held frame before the death point to a hold and
    // discard everything after it, so the next attempt tries a jump one step
    // earlier than the last change.
    int start = std::min(info.frame, static_cast<int>(m_holds.size())) - 1;
    for (int i = start; i >= 0; --i) {
        if (!m_holds[static_cast<std::size_t>(i)]) {
            m_holds[static_cast<std::size_t>(i)] = true;
            m_holds.resize(static_cast<std::size_t>(i) + 1);
            return;
        }
    }
    m_exhausted = true;
}

} // namespace dashback
