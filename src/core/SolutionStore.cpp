#include "SolutionStore.hpp"

#include <Geode/Geode.hpp>

#include <filesystem>
#include <fstream>

using namespace geode::prelude;

namespace dashback {

static std::filesystem::path pathFor(const std::string& algorithm, int levelID) {
    return Mod::get()->getSaveDir() / "solutions" /
        fmt::format("{}_{}.txt", algorithm, levelID);
}

// File format (compact — solutions are sparse):
//   line 1: <total frame count>
//   line 2: space-separated frame indices where jump is held
std::optional<std::vector<bool>> SolutionStore::load(const std::string& algorithm, int levelID) {
    auto path = pathFor(algorithm, levelID);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) return std::nullopt;

    std::ifstream in(path);
    if (!in) return std::nullopt;

    std::size_t frames = 0;
    if (!(in >> frames) || frames == 0) return std::nullopt;

    std::vector<bool> seq(frames, false);
    std::size_t idx = 0;
    while (in >> idx) {
        if (idx < frames) seq[idx] = true;
    }
    return seq;
}

void SolutionStore::save(const std::string& algorithm, int levelID, const std::vector<bool>& seq) {
    auto path = pathFor(algorithm, levelID);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        log::error("dashback: could not write solution to {}", path.string());
        return;
    }

    out << seq.size() << '\n';
    bool first = true;
    for (std::size_t i = 0; i < seq.size(); ++i) {
        if (!seq[i]) continue;
        out << (first ? "" : " ") << i;
        first = false;
    }
    out << '\n';
    log::info("dashback: saved {}-frame solution -> {}", seq.size(), path.string());
}

void SolutionStore::remove(const std::string& algorithm, int levelID) {
    std::error_code ec;
    std::filesystem::remove(pathFor(algorithm, levelID), ec);
}

} // namespace dashback
