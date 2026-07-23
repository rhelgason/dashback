#pragma once

class PlayerObject;
class GJBaseGameLayer;

namespace dashback {

// Obstacle sensing, built on GD's own spatial queries (damagingObjectsInRect /
// staticObjectsInRect). This is the foundation for future reactive / RL
// algorithms and the sensing HUD; the fitness-guided search algorithms
// (genetic, hill-climbing, annealing) deliberately do NOT use it — they only
// need end-of-run progress. It is computed on demand, so it costs nothing for
// algorithms that never call it.
//
// NOTE: coordinate/lookahead details need in-game calibration; treat distances
// as approximate until validated with the sensing HUD.
struct SenseResult {
    bool hazardAhead = false;
    float hazardDistance = 1e9f; // x-distance from the player to the nearest hazard ahead
    bool solidAhead = false;
    float solidDistance = 1e9f;  // x-distance to the nearest solid ahead
};

// Look `lookahead` units ahead of the player (within +/- vertRadius vertically)
// and report the nearest hazard/solid.
SenseResult sense(GJBaseGameLayer* gameLayer, PlayerObject* player,
    float lookahead, float vertRadius);

} // namespace dashback
