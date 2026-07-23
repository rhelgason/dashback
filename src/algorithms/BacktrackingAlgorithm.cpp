#include "BacktrackingAlgorithm.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>

namespace dashback {

void BacktrackingAlgorithm::onLevelStart(const LevelInfo& /*level*/) {
    m_granularity = std::max(1, static_cast<int>(
        geode::Mod::get()->getSettingValue<int64_t>("search-granularity")));
}

InputState BacktrackingAlgorithm::decide(const StepContext& ctx) {
    int f = ctx.frame;
    bool jump = (f < static_cast<int>(m_committed.size())) && m_committed[f];
    if (f == m_probe) jump = true; // this attempt's trial jump
    return {jump};
}

void BacktrackingAlgorithm::onDeath(const DeathInfo& info) {
    int death = info.frame;

    if (m_probe < 0) {
        // First death: nothing committed yet. Start probing just before it.
        m_bestDeath = death;
        m_probe = death - 1;
        if (m_probe <= m_lastCommitted) m_exhausted = true;
        return;
    }

    if (death > m_bestDeath) {
        // The trial jump at m_probe extended our furthest progress — lock it in.
        if (m_probe >= static_cast<int>(m_committed.size())) {
            m_committed.resize(m_probe + 1, false);
        }
        m_committed[m_probe] = true;
        m_lastCommitted = m_probe; // frontier advances
        m_bestDeath = death;
        m_probe = death - 1; // probe near the new death point
    } else {
        // Trial jump didn't help; try inserting a jump earlier.
        m_probe -= m_granularity;
    }

    // Never probe into already-committed territory: everything up to the last
    // committed jump is final. If we've run out of room, this obstacle can't be
    // cleared by a single extra jump (greedy limitation) — stop.
    if (m_probe <= m_lastCommitted) m_exhausted = true;
}

} // namespace dashback
