#pragma once

#include <algorithm>
#include <random>
#include <vector>

namespace dashback {

// Shared helpers for the fitness-guided sequence-search algorithms (genetic,
// hill-climbing, annealing). The genome is a per-frame hold bitstring, which is
// mode-agnostic: the same bit drives cube taps, ship thrust, wave, etc.
namespace seq {

using Genome = std::vector<bool>;

// Grow `g` to at least `len`, filling new frames with random holds at `holdProb`
// (so exploration extends past where the genome currently reaches).
inline void ensureLen(Genome& g, std::size_t len, std::mt19937& rng, double holdProb) {
    std::bernoulli_distribution hold(holdProb);
    while (g.size() < len) g.push_back(hold(rng));
}

// Death-focused mutation: flip bits in a window centered on `center` (the frame
// the run died on) with probability `flipProb`, and make sure the genome reaches
// a bit past the window so it can explore forward. This is why the search
// improves quickly — it perturbs where the failure is, not the working prefix.
inline void mutateAround(Genome& g, int center, int window, double flipProb,
    double holdProb, std::mt19937& rng) {
    int hi = center + window;
    ensureLen(g, static_cast<std::size_t>(std::max(0, hi)), rng, holdProb);
    int lo = std::max(0, center - window);
    std::bernoulli_distribution flip(flipProb);
    for (int f = lo; f < hi && f < static_cast<int>(g.size()); ++f) {
        if (flip(rng)) g[f] = !g[f];
    }
}

// Single-point crossover; child length is the longer parent's, tail taken from
// whichever parent is longer past the cut.
inline Genome crossover(const Genome& a, const Genome& b, std::mt19937& rng) {
    std::size_t len = std::max(a.size(), b.size());
    Genome child;
    child.reserve(len);
    std::size_t cut = len ? std::uniform_int_distribution<std::size_t>(0, len)(rng) : 0;
    for (std::size_t i = 0; i < len; ++i) {
        const Genome& src = (i < cut) ? a : b;
        child.push_back(i < src.size() ? src[i] : false);
    }
    return child;
}

} // namespace seq
} // namespace dashback
