# Changelog

## 1.1.0
- New algorithms: **genetic** (population of hold-sequences, tournament +
  crossover + death-focused mutation), **hillclimb** ((1+1) local search), and
  **annealing** (hill-climb that accepts worse candidates with a cooling
  probability). Select via the Algorithm setting.
- Game-mode detection: cube/ship/ball/UFO/wave/robot/spider/swing, shown on the
  HUD. The per-frame hold representation is already correct for every mode; the
  mode is exposed for awareness and future mode-biased search.
- Obstacle-sensing foundation (`Perception`) built on GD's damagingObjectsInRect
  / staticObjectsInRect, with a "Show Obstacle Sensing" debug HUD toggle to
  validate it. Not used by the search algorithms (staged for future reactive/RL
  work); computed on demand so it costs nothing otherwise.
- Settings for population size, mutation rate, mutation window, and annealing
  temperature.

## 1.0.8
- Backtracking now searches over variable-length HOLD intervals instead of
  single-frame taps, so it can express sustained input (flight/ship sections,
  held/consecutive jumps) — not just isolated taps. Still pure backtracking:
  systematic length-major enumeration with branch-and-bound pruning and undo, no
  game-state heuristics or mode-awareness. New "Max Hold Length" setting bounds a
  single held input; "Search Granularity" now steps both hold start and length.

## 1.0.7
- True depth-first backtracking with undo: keeps a stack of committed jump
  decisions and, when a branch dead-ends, pops the last jump and tries a
  different one for that slot — instead of greedily locking in the first
  progress-extending jump and wedging. Much more robust on obstacles where the
  death happens well after the actual mistake.

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
