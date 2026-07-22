#include "Builtins.hpp"

#include "../core/AlgorithmRegistry.hpp"
#include "BacktrackingAlgorithm.hpp"
#include "GeneticAlgorithm.hpp"
#include "RandomSearchAlgorithm.hpp"

#include <memory>

namespace dashback {

void registerBuiltinAlgorithms() {
    static bool done = false;
    if (done) return;
    done = true;

    auto& registry = AlgorithmRegistry::get();
    registry.reg("backtracking", [] { return std::make_unique<BacktrackingAlgorithm>(); });
    registry.reg("random", [] { return std::make_unique<RandomSearchAlgorithm>(); });
    registry.reg("genetic", [] { return std::make_unique<GeneticAlgorithm>(); });
}

} // namespace dashback
