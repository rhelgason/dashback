# dashback ML pipeline

Offline training for the in-game `policy` algorithm. The loop is:

1. **Collect data** — in GD, enable the **Record Training Data** setting and run
   any solver (backtracking / genetic / …). Solved and new-best runs are logged
   as per-frame `(observation, action)` rows to:
   `<mod save dir>/trajectories/<algorithm>_<levelID>.csv`
   (On CrossOver: `…/Geometry Dash/geode/config/rhelgason.dashback/` or the mod's
   save dir — the log line `logged … trajectory -> <path>` prints the exact path.)
2. **Copy the CSVs** into `ml/data/` here.
3. **Train** — `python train.py --data data --out model.txt` produces an MLP in
   the format the mod loads.
4. **Install the model** — copy `model.txt` to `<mod save dir>/policy/model.txt`.
5. **Run it** — set the **Algorithm** setting to `policy`. It senses each frame
   and plays the network's action.

## Observation schema

The feature vector is defined once in C++ (`src/core/Observation.*`) and mirrored
in `observation_schema.md`. `train.py` reads the CSV header, so as long as the
columns match it stays in sync. Current layout (25 features):

- `hazard0..6` — normalized x-distance to the nearest hazard ahead, per vertical band (1 = none)
- `solid0..6`  — same for solid objects
- `yvel`, `onground`, `upsidedown`
- `mode0..7`   — one-hot game mode (cube/ship/ball/UFO/wave/robot/spider/swing)

Label column: `action` (1 = hold that frame). Extra columns (`solved`, `session`,
`level_id`) are metadata for filtering/weighting, not model inputs.

## Notes / honest status

- This is **imitation learning** (behavior cloning): predict the action the
  successful/furthest run took, given the observation. It's the simplest path and
  the data is free. RL can come later on the same observation.
- Everything here is **gated on perception being correct** — validate it in-game
  with the **Show Obstacle Sensing** HUD toggle before trusting a trained model.
- The realistic near-term target is **ML-as-a-prior**: use the policy to
  warm-start the genetic search (far fewer attempts), not to solve outright. The
  standalone `policy` algorithm is the first step toward that.

## Weight file format (`model.txt`)

```
DASHBACK_MLP 1
<F> <H>
<W1: H*F floats>  <b1: H>  <W2: H*H>  <b2: H>  <W3: H>  <b3: 1>
```
Whitespace-separated; a two-hidden-layer MLP `F -> H -> H -> 1` with ReLU hidden
activations and a sigmoid output. `F` must equal the observation feature count.
