#pragma once

namespace dashback {

// Registers all built-in algorithms with the AlgorithmRegistry. Called once by
// the SolverController (idempotent). Register new algorithms here so they become
// selectable via the "algorithm" setting.
void registerBuiltinAlgorithms();

} // namespace dashback
