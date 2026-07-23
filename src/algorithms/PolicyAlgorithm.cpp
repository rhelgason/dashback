#include "PolicyAlgorithm.hpp"

#include "../core/Observation.hpp"

#include <Geode/Geode.hpp>

#include <cmath>
#include <fstream>

using namespace geode::prelude;

namespace dashback {

// Weight file format (produced by ml/train.py):
//   DASHBACK_MLP 1
//   <F> <H>
//   W1 (H*F)  b1 (H)  W2 (H*H)  b2 (H)  W3 (H)  b3 (1)     [whitespace-separated]
bool PolicyAlgorithm::loadModel() {
    auto path = Mod::get()->getSaveDir() / "policy" / "model.txt";
    std::ifstream in(path);
    if (!in) return false;

    std::string magic;
    int version = 0;
    in >> magic >> version;
    if (magic != "DASHBACK_MLP") return false;

    in >> m_inDim >> m_hidden;
    if (m_inDim <= 0 || m_hidden <= 0) return false;
    if (m_inDim != Observation::kFeatureCount) {
        log::warn("dashback[policy]: model expects {} features but observation has {}",
            m_inDim, Observation::kFeatureCount);
        return false;
    }

    auto readN = [&](std::vector<float>& v, int n) {
        v.resize(n);
        for (int i = 0; i < n; ++i) in >> v[i];
    };
    readN(m_w1, m_hidden * m_inDim);
    readN(m_b1, m_hidden);
    readN(m_w2, m_hidden * m_hidden);
    readN(m_b2, m_hidden);
    readN(m_w3, m_hidden);
    in >> m_b3;

    return static_cast<bool>(in) || in.eof();
}

float PolicyAlgorithm::forward(const std::vector<float>& x) const {
    std::vector<float> h1(m_hidden), h2(m_hidden);
    for (int j = 0; j < m_hidden; ++j) {
        float s = m_b1[j];
        for (int i = 0; i < m_inDim; ++i) s += m_w1[j * m_inDim + i] * x[i];
        h1[j] = s > 0.f ? s : 0.f; // relu
    }
    for (int j = 0; j < m_hidden; ++j) {
        float s = m_b2[j];
        for (int i = 0; i < m_hidden; ++i) s += m_w2[j * m_hidden + i] * h1[i];
        h2[j] = s > 0.f ? s : 0.f;
    }
    float out = m_b3;
    for (int j = 0; j < m_hidden; ++j) out += m_w3[j] * h2[j];
    return 1.f / (1.f + std::exp(-out)); // sigmoid
}

void PolicyAlgorithm::onLevelStart(const LevelInfo& /*level*/) {
    m_loaded = loadModel();
    m_played.clear();
    m_done = false;
    if (m_loaded) {
        log::info("dashback[policy]: loaded model ({} features, {} hidden)", m_inDim, m_hidden);
    } else {
        log::warn("dashback[policy]: no usable model at <save>/policy/model.txt — "
                  "policy is inert. Train one with ml/train.py.");
    }
}

void PolicyAlgorithm::onAttemptStart(int /*attempt*/) {
    m_played.clear();
}

InputState PolicyAlgorithm::decide(const StepContext& ctx) {
    bool hold = false;
    if (m_loaded && ctx.gameLayer && ctx.player) {
        Observation obs = observe(ctx.gameLayer, ctx.player,
            Observation::kLookahead, Observation::kBandHeight);
        hold = forward(obs.toFeatures()) > 0.5f;
    }
    m_played.push_back(hold);
    return {hold};
}

void PolicyAlgorithm::onDeath(const DeathInfo& /*info*/) {
    // Deterministic — retrying reproduces the same run, so stop after one pass.
    m_done = true;
}

} // namespace dashback
