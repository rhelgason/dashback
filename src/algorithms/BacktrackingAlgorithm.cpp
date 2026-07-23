#include "BacktrackingAlgorithm.hpp"

namespace dashback {

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
        // Trial jump didn't help; try inserting a jump one frame earlier.
        --m_probe;
    }

    if (m_probe < 0) m_exhausted = true; // ran out of frames to probe
}

} // namespace dashback
