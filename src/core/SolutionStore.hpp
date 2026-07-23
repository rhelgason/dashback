#pragma once

#include <optional>
#include <string>
#include <vector>

namespace dashback {

// Persists a completed level's full per-frame input sequence, keyed by
// (algorithm, levelID), so it can be replayed end-to-end on later visits.
// Stored as a small text file under <save dir>/solutions/.
class SolutionStore {
public:
    static std::optional<std::vector<bool>> load(const std::string& algorithm, int levelID);
    static void save(const std::string& algorithm, int levelID, const std::vector<bool>& seq);
    static void remove(const std::string& algorithm, int levelID);
};

} // namespace dashback
