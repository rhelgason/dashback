#pragma once

#include "Types.hpp"

#include <string>

namespace dashback {

// Appends one CSV row per attempt to <mod save dir>/metrics.csv. This is the
// backbone of the whole project's goal: comparing runtime, deaths, and success
// rate across algorithms. Every attempt of every algorithm lands in one file
// you can load into a spreadsheet / pandas and slice by the `algorithm` column.
class MetricsRecorder {
public:
    // `sessionId` groups all attempts from one level-open together so you can
    // tell separate solve runs apart in the CSV.
    void begin(const LevelInfo& level, const std::string& algorithm, long long sessionId);

    // Record the outcome of a single attempt.
    void record(const AttemptResult& result);

    // Path of the CSV file (for logging / surfacing to the user).
    std::string filePath() const;

private:
    void ensureHeader();

    LevelInfo m_level;
    std::string m_algorithm;
    long long m_sessionId = 0;
    bool m_headerChecked = false;
};

} // namespace dashback
