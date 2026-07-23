# Changelog

## 1.0.2
- Fix death detection: GD calls `destroyPlayer` during the pre-start intro and
  for non-lethal contacts where it early-returns without killing. Only count a
  death when the player's `m_isDead` actually becomes true — the solver no longer
  "dies" during the intro and gives up.
- Rework the backtracking algorithm to commit progress (greedy jump-insertion)
  instead of brute-forcing the pattern right before the death point; it now makes
  visible, monotonic progress.
- HUD shows all-time best progress (not just the current attempt's).

## 1.0.1
- Port to Geode 5.8.2 / Geometry Dash 2.2081. `GJBaseGameLayer::processCommands`
  gained `isHalfTick`/`isLastTick` parameters in 2.2081; the hook now matches and
  passes them through (still counting every invocation as a deterministic step).

## 1.0.0
- Deterministic input/replay foundation: hook `GJBaseGameLayer::processCommands`
  (per physics step) instead of counting rendered frames, and apply input via the
  game's `handleButton`. Fixes the prototype's replay desync.
- Pluggable algorithm framework: `Algorithm` interface + `AlgorithmRegistry`,
  driven by a `SolverController`.
- Three built-in algorithms: `backtracking`, `random`, `genetic`.
- Per-attempt metrics logged to `metrics.csv` (algorithm, success, progress,
  frames, wall time) for cross-algorithm comparison.
- On-screen HUD (per-instance, no dangling pointers) showing algorithm, attempt,
  and best progress.
- Settings for enabling the solver, choosing the algorithm, capping attempts, and
  the (experimental) daily auto-solver.
