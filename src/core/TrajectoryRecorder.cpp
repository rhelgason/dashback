#include "TrajectoryRecorder.hpp"

#include "Observation.hpp"

#include <Geode/Geode.hpp>

#include <filesystem>
#include <fstream>

using namespace geode::prelude;

namespace dashback {

std::string TrajectoryRecorder::filePath() const {
    auto dir = Mod::get()->getSaveDir() / "trajectories";
    return (dir / fmt::format("{}_{}.csv", m_algorithm, m_level.levelID)).string();
}

void TrajectoryRecorder::begin(const LevelInfo& level, const std::string& algorithm, long long sessionId) {
    m_level = level;
    m_algorithm = algorithm;
    m_sessionId = sessionId;
    m_buffer.clear();
    m_headerWritten = false;
}

void TrajectoryRecorder::record(const std::vector<float>& features, bool action) {
    if (!m_enabled) return;
    m_buffer.push_back(Row{features, action});
}

void TrajectoryRecorder::endAttempt(bool solved, bool newBest, int reachedFrame) {
    if (!m_enabled) { m_buffer.clear(); return; }
    // Only keep runs we can learn from: a full solve, or a new furthest reach.
    if (solved || newBest) flush(solved, reachedFrame);
    m_buffer.clear();
}

void TrajectoryRecorder::flush(bool solved, int reachedFrame) {
    if (m_buffer.empty()) return;
    auto path = std::filesystem::path(filePath());
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    bool needHeader = !std::filesystem::exists(path, ec) ||
        std::filesystem::file_size(path, ec) == 0;

    std::ofstream out(path, std::ios::app);
    if (!out) {
        log::error("dashback: could not write trajectory to {}", path.string());
        return;
    }

    if (needHeader) {
        for (auto const& name : Observation::featureNames()) out << name << ',';
        out << "action,solved,session,level_id\n";
    }

    for (auto const& row : m_buffer) {
        for (float v : row.features) out << fmt::format("{:.4f}", v) << ',';
        out << (row.action ? 1 : 0) << ',' << (solved ? 1 : 0) << ','
            << m_sessionId << ',' << m_level.levelID << '\n';
    }
    log::info("dashback: logged {}-frame {} trajectory (reached {}) -> {}",
        m_buffer.size(), solved ? "SOLVED" : "progress", reachedFrame, path.string());
}

} // namespace dashback
