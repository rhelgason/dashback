#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

namespace dashback {

// Turns runs into imitation-learning data. During an attempt it buffers
// (observation-features, action) per frame; when the attempt ends it writes the
// buffer to a CSV IF the attempt is worth learning from — a completed level, or
// a new best-progress demonstration — and discards it otherwise. So the search
// solvers double as a dataset generator: their successful/furthest runs become
// labeled trajectories for the offline trainer.
//
// Opt-in (record-trajectories setting), because it computes an observation every
// frame; off by default so it never slows normal solving.
class TrajectoryRecorder {
public:
    void begin(const LevelInfo& level, const std::string& algorithm, long long sessionId);
    void setEnabled(bool on) { m_enabled = on; }
    bool enabled() const { return m_enabled; }

    void record(const std::vector<float>& features, bool action); // buffer one frame
    void endAttempt(bool solved, bool newBest, int reachedFrame); // flush or discard
    void discard() { m_buffer.clear(); }

private:
    void flush(bool solved, int reachedFrame);
    std::string filePath() const;

    struct Row { std::vector<float> features; bool action; };
    std::vector<Row> m_buffer;
    LevelInfo m_level;
    std::string m_algorithm;
    long long m_sessionId = 0;
    bool m_enabled = false;
    bool m_headerWritten = false;
};

} // namespace dashback
