#include <Geode/Geode.hpp>

using namespace geode::prelude;

// dashback — an algorithmic Geometry Dash solver.
//
// All behaviour is installed by the $modify hooks under src/hooks/, which drive
// the SolverController (src/core/). The controller runs the algorithm selected
// by the "algorithm" setting and logs per-attempt metrics for comparison.
//
// See README.md for the architecture and how to add a new algorithm.
$on_mod(Loaded) {
    log::info("dashback loaded — algorithmic Geometry Dash solver");
}
