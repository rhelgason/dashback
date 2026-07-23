#include "Builtins.hpp"

#include "../core/AlgorithmRegistry.hpp"
#include "BacktrackingAlgorithm.hpp"
#include "GeneticAlgorithm.hpp"
#include "HillClimbingAlgorithm.hpp"
#include "PolicyAlgorithm.hpp"
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
    registry.reg("hillclimb", [] { return std::make_unique<HillClimbingAlgorithm>(false); });
    registry.reg("annealing", [] { return std::make_unique<HillClimbingAlgorithm>(true); });
    registry.reg("policy", [] { return std::make_unique<PolicyAlgorithm>(); });
}

} // namespace dashback
