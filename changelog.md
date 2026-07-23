# Changelog

## 1.0.6
- Remove checkpoint fast-restart. GD's loadFromCheckpoint is a practice-mode
  facility; used standalone it either failed to restore (replayed from frame 0)
  or left the game hung with no respawn. Solving now always uses reliable full
  resets — kept fast by Solve Speed.
- Solve Speed default 6x, max raised to 20x; adjust live with [ and ].

## 1.0.5
- Fix checkpoint crash: `createCheckpoint()` returns an autoreleased object; we
  now retain our reference (and release it) so `loadFromCheckpoint` can't
  dereference freed memory. Fixes the access violation in loadFromCheckpoint.
- Fast Restart now defaults OFF (opt-in). With it off, solving uses reliable full
  resets — still fast via Solve Speed. Turn it on to test the checkpoint path.

## 1.0.4
- Checkpoint fast-restart (setting "Fast Restart", default on): during search,
  snapshot state at the committed frontier and restart there instead of from
  frame 0, so each attempt only replays to the next obstacle. Removes the
  linear per-attempt slowdown deep in a level.
- Solution storage + end-to-end replay: when an algorithm completes a level, its
  full frame-0 input sequence is saved (per algorithm + level). Reopening the
  level replays that sequence start-to-finish at watchable speed — a genuine E2E
  run. If a stored solution ever fails to complete, it's discarded and re-searched.
- Live speed control: `]` faster, `[` slower (0.5x–12x), shown on the HUD.

## 1.0.3
- Add "Solve Speed" setting (default 4x): speeds up iteration via the scheduler
  time scale. Physics is fixed-step so this doesn't affect determinism. Reset to
  1x on leaving the level.
- Add "Search Granularity" setting (default 1 = every frame): how many frames the
  backtracking probe steps at a time. Higher scans faster but can miss
  frame-perfect jumps.

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
