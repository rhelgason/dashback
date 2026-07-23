#include "BacktrackingAlgorithm.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>

namespace dashback {

void BacktrackingAlgorithm::onLevelStart(const LevelInfo& /*level*/) {
    m_granularity = std::max(1, static_cast<int>(
        geode::Mod::get()->getSettingValue<int64_t>("search-granularity")));
    m_maxHold = std::max(1, static_cast<int>(
        geode::Mod::get()->getSettingValue<int64_t>("max-hold-length")));
    m_committed.clear();
    m_stack.clear();
    m_baseDeath = -1;
    m_lowerBound = 0;
    m_searchTop = -1;
    m_probeStart = -1;
    m_probeLen = 0;
    m_exhausted = false;
}

void BacktrackingAlgorithm::setHold(int start, int len, bool on) {
    if (start < 0 || len <= 0) return;
    int end = start + len;
    if (end > static_cast<int>(m_committed.size())) m_committed.resize(end, false);
    for (int f = start; f < end; ++f) m_committed[f] = on;
}

InputState BacktrackingAlgorithm::decide(const StepContext& ctx) {
    int f = ctx.frame;
    bool hold = (f >= 0 && f < static_cast<int>(m_committed.size()) && m_committed[f]);
    if (m_probeStart >= 0 && f >= m_probeStart && f < m_probeStart + m_probeLen) hold = true;
    return {hold};
}

// Length-major enumeration: sweep start from searchTop down to lowerBound for a
// fixed length, then increase the length. Returns false when the slot is spent.
bool BacktrackingAlgorithm::advanceProbe() {
    m_probeStart -= m_granularity;
    if (m_probeStart < m_lowerBound) {
        m_probeStart = m_searchTop;
        m_probeLen += m_granularity;
        if (m_probeLen > m_maxHold) return false;
    }
    return m_probeStart >= m_lowerBound;
}

void BacktrackingAlgorithm::backtrack() {
    while (!m_stack.empty()) {
        Decision d = m_stack.back();
        m_stack.pop_back();
        setHold(d.start, d.len, false); // undo this hold
        m_baseDeath = d.baseDeath;
        m_lowerBound = d.lowerBound;
        m_searchTop = d.searchTop;
        m_probeStart = d.resumeStart;
        m_probeLen = d.resumeLen;
        if (m_probeLen <= m_maxHold && m_probeStart >= m_lowerBound) return;
    }
    m_exhausted = true;
}

void BacktrackingAlgorithm::onDeath(const DeathInfo& info) {
    int death = info.frame;

    // Establish (or re-establish) how far the current committed set gets, then
    // begin probing hold intervals just before the death.
    if (m_baseDeath < 0 || m_probeStart < 0) {
        m_baseDeath = death;
        m_searchTop = death - 1;
        m_probeLen = 1;
        m_probeStart = m_searchTop;
        if (m_probeStart < m_lowerBound) backtrack();
        return;
    }

    if (death > m_baseDeath) {
        // This hold extended our furthest progress — commit it as a decision.
        int rStart = m_probeStart - m_granularity;
        int rLen = m_probeLen;
        if (rStart < m_lowerBound) { rStart = m_searchTop; rLen += m_granularity; }
        m_stack.push_back(Decision{m_probeStart, m_probeLen, m_baseDeath,
            m_lowerBound, m_searchTop, rStart, rLen});
        setHold(m_probeStart, m_probeLen, true);

        m_lowerBound = m_probeStart + m_probeLen; // next hold begins after this one
        m_baseDeath = death;
        m_searchTop = death - 1;
        m_probeLen = 1;
        m_probeStart = m_searchTop;
        if (m_probeStart < m_lowerBound) backtrack();
    } else {
        // Didn't help — try the next hold interval.
        if (!advanceProbe()) backtrack();
    }
}

void BacktrackingAlgorithm::onComplete(const AttemptResult& /*result*/) {
    // Fold the winning trial hold into the committed sequence so solution() is
    // the full input that finished the level.
    if (m_probeStart >= 0) setHold(m_probeStart, m_probeLen, true);
}

} // namespace dashback
