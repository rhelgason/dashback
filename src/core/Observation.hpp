#pragma once

#include <array>
#include <string>
#include <vector>

class PlayerObject;
class GJBaseGameLayer;

namespace dashback {

// A fixed-size, normalized view of the world around the player — the input a
// learned policy (or a reactive controller) consumes, and the thing we log as
// training data. The layout is the single source of truth shared with the
// offline trainer (see ml/observation_schema.md); keep toFeatures() and the
// schema doc in sync.
struct Observation {
    static constexpr int kBands = 7;   // vertical bands ahead, centered on the player
    static constexpr int kModes = 8;   // GameMode count
    static constexpr int kFeatureCount = kBands * 2 + 3 + kModes; // 25

    // Sensing framing — MUST be identical for logging and inference so the
    // trained model sees the same observation it was trained on.
    static constexpr float kLookahead = 300.f;
    static constexpr float kBandHeight = 30.f;

    std::array<float, kBands> hazardDist{}; // nearest hazard ahead per band, 0..1 (1 = none)
    std::array<float, kBands> solidDist{};  // nearest solid ahead per band, 0..1 (1 = none)
    float yVelocity = 0.f;                  // normalized
    float onGround = 0.f;                   // 0/1
    float upsideDown = 0.f;                 // 0/1
    int mode = 0;                           // GameMode index (0..kModes-1)

    // Flatten to the fixed feature vector the model consumes.
    std::vector<float> toFeatures() const;

    // Column names for the training CSV header (size kFeatureCount).
    static std::vector<std::string> featureNames();
};

// Build an observation by sensing objects ahead of the player, bucketed into
// vertical bands. `lookahead` is how far ahead to look; `bandHeight` is the
// vertical size of each band. Uses GD's damaging/static object queries.
Observation observe(GJBaseGameLayer* gameLayer, PlayerObject* player,
    float lookahead, float bandHeight);

} // namespace dashback
