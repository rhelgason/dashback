#include "BacktrackingAlgorithm.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>

namespace dashback {

void BacktrackingAlgorithm::onLevelStart(const LevelInfo& /*level*/) {
    m_granularity = std::max(1, static_cast<int>(
        geode::Mod::get()->getSettingValue<int64_t>("search-granularity")));
    m_committed.clear();
    m_stack.clear();
    m_baseDeath = -1;
    m_lowerBound = 0;
    m_probe = -1;
    m_exhausted = false;
}

void BacktrackingAlgorithm::setJump(int frame, bool on) {
    if (frame < 0) return;
    if (frame >= static_cast<int>(m_committed.size())) m_committed.resize(frame + 1, false);
    m_committed[frame] = on;
}

InputState BacktrackingAlgorithm::decide(const StepContext& ctx) {
    int f = ctx.frame;
    bool jump = (f >= 0 && f < static_cast<int>(m_committed.size()) && m_committed[f]);
    if (f == m_probe) jump = true; // this attempt's trial jump
    return {jump};
}

void BacktrackingAlgorithm::backtrack() {
    // The current slot's candidates are exhausted. Undo committed decisions until
    // one still has an earlier candidate to try; if none, the search is done.
    while (!m_stack.empty()) {
        Decision d = m_stack.back();
        m_stack.pop_back();
        setJump(d.jumpFrame, false); // undo this jump
        m_baseDeath = d.baseDeath;
        m_lowerBound = d.lowerBound;
        m_probe = d.nextProbe;
        if (m_probe >= m_lowerBound) return; // found a slot with an untried candidate
    }
    m_exhausted = true; // nothing left to try
}

void BacktrackingAlgorithm::onDeath(const DeathInfo& info) {
    int death = info.frame;

    // First death (or re-establishing after we somehow lost the baseline):
    // record how far the current committed set gets, then start probing.
    if (m_baseDeath < 0 || m_probe < 0) {
        m_baseDeath = death;
        m_probe = death - 1;
        if (m_probe < m_lowerBound) backtrack();
        return;
    }

    if (death > m_baseDeath) {
        // The trial jump extended our furthest progress — commit it as a decision.
        m_stack.push_back(Decision{m_probe, m_baseDeath, m_lowerBound, m_probe - m_granularity});
        setJump(m_probe, true);
        m_lowerBound = m_probe + 1; // future jumps for the next slot come after this one
        m_baseDeath = death;
        m_probe = death - 1;        // probe just before the new death point
        if (m_probe < m_lowerBound) backtrack();
    } else {
        // Didn't help — try inserting the jump earlier.
        m_probe -= m_granularity;
        if (m_probe < m_lowerBound) backtrack();
    }
}

void BacktrackingAlgorithm::onComplete(const AttemptResult& /*result*/) {
    // The completing run included the current trial jump; fold it into the
    // committed sequence so solution() is the full winning input.
    if (m_probe >= 0) setJump(m_probe, true);
}

} // namespace dashback
