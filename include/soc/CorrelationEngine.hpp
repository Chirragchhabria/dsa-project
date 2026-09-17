#pragma once

#include "soc/EntityGraph.hpp"
#include "soc/EventManager.hpp"
#include "soc/Incident.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace soc {

struct CorrelationConfig {
    std::int64_t windowSeconds{300};
    std::size_t minimumIncidentEvents{2};
};

class CorrelationEngine {
public:
    explicit CorrelationEngine(CorrelationConfig config = {});
    std::vector<Incident> correlate(const EventManager& manager);

    const EntityGraph& graph() const noexcept { return graph_; }

private:
    bool sharesEntity(const SecurityEvent& first, const SecurityEvent& second) const;
    bool typesRelated(EventType first, EventType second) const;

    CorrelationConfig config_;
    EntityGraph graph_;
};

}  // namespace soc

