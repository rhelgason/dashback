#include "MetricsRecorder.hpp"

#include <Geode/Geode.hpp>

#include <filesystem>
#include <fstream>

using namespace geode::prelude;

namespace dashback {

static std::filesystem::path metricsPath() {
    return Mod::get()->getSaveDir() / "metrics.csv";
}

// CSV-escape a field (level names can contain commas / quotes).
static std::string csvField(const std::string& in) {
    bool needsQuote = in.find_first_of(",\"\n") != std::string::npos;
    if (!needsQuote) return in;
    std::string out = "\"";
    for (char c : in) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += "\"";
    return out;
}

void MetricsRecorder::begin(const LevelInfo& level, const std::string& algorithm, long long sessionId) {
    m_level = level;
    m_algorithm = algorithm;
    m_sessionId = sessionId;
    m_headerChecked = false;
    ensureHeader();
}

void MetricsRecorder::ensureHeader() {
    if (m_headerChecked) return;
    m_headerChecked = true;

    auto path = metricsPath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    bool exists = std::filesystem::exists(path, ec);
    if (exists && std::filesystem::file_size(path, ec) > 0) return;

    std::ofstream out(path, std::ios::app);
    if (!out) {
        log::error("dashback: could not open metrics file {}", path.string());
        return;
    }
    out << "session_id,algorithm,level_id,level_name,attempt,success,"
           "best_progress,death_frame,frames,wall_ms\n";
}

void MetricsRecorder::record(const AttemptResult& r) {
    ensureHeader();

    std::ofstream out(metricsPath(), std::ios::app);
    if (!out) {
        log::error("dashback: could not append to metrics file");
        return;
    }

    out << m_sessionId << ','
        << csvField(m_algorithm) << ','
        << m_level.levelID << ','
        << csvField(m_level.name) << ','
        << r.attempt << ','
        << (r.success ? 1 : 0) << ','
        << fmt::format("{:.4f}", r.bestProgress) << ','
        << r.deathFrame << ','
        << r.frames << ','
        << fmt::format("{:.1f}", r.wallMs) << '\n';
}

std::string MetricsRecorder::filePath() const {
    return metricsPath().string();
}

} // namespace dashback
