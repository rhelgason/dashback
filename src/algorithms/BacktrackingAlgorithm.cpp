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
        // First death: nothing committed yet. Start probing just before the point
        // where we died.
        m_bestDeath = death;
        m_probe = death - 1;
        if (m_probe < 0) m_exhausted = true;
        return;
    }

    if (death > m_bestDeath) {
        // The trial jump at m_probe extended our furthest progress — lock it in.
        if (m_probe >= static_cast<int>(m_committed.size())) {
            m_committed.resize(m_probe + 1, false);
        }
        m_committed[m_probe] = true;
        m_bestDeath = death;
        m_probe = death - 1; // now probe near the new death point
    } else {
        // Trial jump didn't help; try inserting a jump earlier (step by the
        // configured granularity).
        m_probe -= m_granularity;
    }

    if (m_probe < 0) m_exhausted = true; // ran out of frames to probe
}

} // namespace dashback
