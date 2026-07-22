# Changelog

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
