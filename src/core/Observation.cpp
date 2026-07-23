#include "Observation.hpp"

#include "Types.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace dashback {

static int modeIndex(PlayerObject* p) {
    if (!p) return static_cast<int>(GameMode::Cube);
    if (p->m_isSwing) return static_cast<int>(GameMode::Swing);
    if (p->m_isSpider) return static_cast<int>(GameMode::Spider);
    if (p->m_isRobot) return static_cast<int>(GameMode::Robot);
    if (p->m_isBall) return static_cast<int>(GameMode::Ball);
    if (p->m_isBird) return static_cast<int>(GameMode::UFO);
    if (p->m_isDart) return static_cast<int>(GameMode::Wave);
    if (p->m_isShip) return static_cast<int>(GameMode::Ship);
    return static_cast<int>(GameMode::Cube);
}

// Fill per-band nearest-ahead distances (normalized) from a query array.
static void bucketByBand(cocos2d::CCArray* objects, float px, float py,
    float lookahead, float bandHeight, std::array<float, Observation::kBands>& out) {
    out.fill(1.f); // 1 = nothing sensed in this band
    if (!objects) return;
    constexpr int half = Observation::kBands / 2;
    for (int i = 0; i < objects->count(); ++i) {
        auto* obj = static_cast<GameObject*>(objects->objectAtIndex(i));
        if (!obj) continue;
        float dx = obj->getPositionX() - px;
        if (dx < 0.f || dx > lookahead) continue;
        int band = static_cast<int>(std::lround((obj->getPositionY() - py) / bandHeight)) + half;
        if (band < 0 || band >= Observation::kBands) continue;
        float norm = dx / lookahead;
        if (norm < out[band]) out[band] = norm;
    }
}

Observation observe(GJBaseGameLayer* gl, PlayerObject* player,
    float lookahead, float bandHeight) {
    Observation obs;
    if (!gl || !player) return obs;

    float px = player->getPositionX();
    float py = player->getPositionY();
    float halfH = (Observation::kBands / 2 + 0.5f) * bandHeight;
    cocos2d::CCRect rect(px, py - halfH, lookahead, halfH * 2.f);

    bucketByBand(gl->damagingObjectsInRect(rect, true), px, py, lookahead, bandHeight, obs.hazardDist);
    bucketByBand(gl->staticObjectsInRect(rect, true), px, py, lookahead, bandHeight, obs.solidDist);

    obs.yVelocity = std::clamp(static_cast<float>(player->m_yVelocity) / 20.f, -1.f, 1.f);
    obs.onGround = player->m_isOnGround ? 1.f : 0.f;
    obs.upsideDown = player->m_isUpsideDown ? 1.f : 0.f;
    obs.mode = modeIndex(player);
    return obs;
}

std::vector<float> Observation::toFeatures() const {
    std::vector<float> f;
    f.reserve(kFeatureCount);
    for (float v : hazardDist) f.push_back(v);
    for (float v : solidDist) f.push_back(v);
    f.push_back(yVelocity);
    f.push_back(onGround);
    f.push_back(upsideDown);
    for (int m = 0; m < kModes; ++m) f.push_back(m == mode ? 1.f : 0.f);
    return f;
}

std::vector<std::string> Observation::featureNames() {
    std::vector<std::string> n;
    for (int i = 0; i < kBands; ++i) n.push_back("hazard" + std::to_string(i));
    for (int i = 0; i < kBands; ++i) n.push_back("solid" + std::to_string(i));
    n.push_back("yvel");
    n.push_back("onground");
    n.push_back("upsidedown");
    for (int i = 0; i < kModes; ++i) n.push_back("mode" + std::to_string(i));
    return n;
}

} // namespace dashback
