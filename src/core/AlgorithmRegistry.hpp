#pragma once

#include "Algorithm.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dashback {

using AlgorithmFactory = std::function<std::unique_ptr<Algorithm>()>;

// A tiny name -> factory registry so the solver can create algorithms by their
// string id (from mod settings) without hard-coding the concrete types. Built-in
// algorithms are registered in algorithms/Builtins.cpp.
class AlgorithmRegistry {
public:
    static AlgorithmRegistry& get();

    void reg(const std::string& id, AlgorithmFactory factory);
    bool has(const std::string& id) const;
    std::unique_ptr<Algorithm> create(const std::string& id) const;
    std::vector<std::string> ids() const;

private:
    std::map<std::string, AlgorithmFactory> m_factories;
};

} // namespace dashback
