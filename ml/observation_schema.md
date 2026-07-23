# Observation schema (mirror of src/core/Observation.*)

25 features, in this exact order (must match `Observation::toFeatures()`):

| index | name        | meaning                                                        |
|------:|-------------|----------------------------------------------------------------|
| 0..6  | hazard0..6  | nearest hazard ahead per vertical band, x-dist / lookahead (1 = none) |
| 7..13 | solid0..6   | nearest solid ahead per vertical band, same normalization      |
| 14    | yvel        | player y-velocity, clamped to [-1, 1]                          |
| 15    | onground    | 1 if on ground else 0                                          |
| 16    | upsidedown  | 1 if gravity is flipped else 0                                 |
| 17..24| mode0..7    | one-hot: cube, ship, ball, UFO, wave, robot, spider, swing     |

Bands are centered on the player: band 3 is level with the player, bands 0..2 are
below, 4..6 above (each `kBandHeight` = 30 units tall). Sensing framing:
`kLookahead = 300`, `kBandHeight = 30`.

Label: `action` — 1 if the button was held that frame.

If you change the C++ observation, update this file and retrain.
