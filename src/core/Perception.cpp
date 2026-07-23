#include "Perception.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>

using namespace geode::prelude;

namespace dashback {

// Nearest object with x >= player x, within the query rect, by x-distance.
static float nearestAheadDistance(cocos2d::CCArray* objects, float playerX) {
    if (!objects) return 1e9f;
    float best = 1e9f;
    for (int i = 0; i < objects->count(); ++i) {
        auto* obj = static_cast<GameObject*>(objects->objectAtIndex(i));
        if (!obj) continue;
        float dx = obj->getPositionX() - playerX;
        if (dx >= 0.f && dx < best) best = dx;
    }
    return best;
}

SenseResult sense(GJBaseGameLayer* gl, PlayerObject* player,
    float lookahead, float vertRadius) {
    SenseResult out;
    if (!gl || !player) return out;

    float px = player->getPositionX();
    float py = player->getPositionY();
    cocos2d::CCRect rect(px, py - vertRadius, lookahead, vertRadius * 2.f);

    out.hazardDistance = nearestAheadDistance(gl->damagingObjectsInRect(rect, true), px);
    out.hazardAhead = out.hazardDistance < lookahead;

    out.solidDistance = nearestAheadDistance(gl->staticObjectsInRect(rect, true), px);
    out.solidAhead = out.solidDistance < lookahead;

    return out;
}

} // namespace dashback
