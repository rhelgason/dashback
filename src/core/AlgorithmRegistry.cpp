#include "AlgorithmRegistry.hpp"

namespace dashback {

AlgorithmRegistry& AlgorithmRegistry::get() {
    static AlgorithmRegistry instance;
    return instance;
}

void AlgorithmRegistry::reg(const std::string& id, AlgorithmFactory factory) {
    m_factories[id] = std::move(factory);
}

bool AlgorithmRegistry::has(const std::string& id) const {
    return m_factories.find(id) != m_factories.end();
}

std::unique_ptr<Algorithm> AlgorithmRegistry::create(const std::string& id) const {
    auto it = m_factories.find(id);
    if (it == m_factories.end()) return nullptr;
    return it->second();
}

std::vector<std::string> AlgorithmRegistry::ids() const {
    std::vector<std::string> out;
    out.reserve(m_factories.size());
    for (auto const& [id, _] : m_factories) out.push_back(id);
    return out;
}

} // namespace dashback
