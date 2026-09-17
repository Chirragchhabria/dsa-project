#include "soc/CorrelationEngine.hpp"

#include "soc/SlidingWindow.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace soc {
namespace {

int eventFamily(EventType type) {
    switch (type) {
        case EventType::FailedLogin:
        case EventType::SuccessfulLogin:
        case EventType::PrivilegeEscalation:
        case EventType::FileAccess:
            return 1;
        case EventType::ConnectionAttempt:
        case EventType::PortScan:
        case EventType::ServiceDiscovery:
            return 2;
        case EventType::PrivilegedAccess:
        case EventType::SensitiveFileAccess:
        case EventType::OutboundConnection:
        case EventType::LargeOutboundTransfer:
            return 3;
        case EventType::NormalActivity:
        case EventType::Unknown:
            return 0;
    }
    return 0;
}

bool sameNonEmpty(const std::string& first, const std::string& second) {
    return !first.empty() && first == second;
}

}  // namespace

CorrelationEngine::CorrelationEngine(CorrelationConfig config) : config_(config) {
    if (config_.windowSeconds <= 0) throw std::invalid_argument("correlation window must be positive");
    if (config_.minimumIncidentEvents < 2) {
        throw std::invalid_argument("minimum incident size must be at least two");
    }
}

bool CorrelationEngine::sharesEntity(const SecurityEvent& first, const SecurityEvent& second) const {
    const bool sharedIp = sameNonEmpty(first.sourceIp, second.sourceIp) ||
        sameNonEmpty(first.sourceIp, second.destinationIp) ||
        sameNonEmpty(first.destinationIp, second.sourceIp) ||
        sameNonEmpty(first.destinationIp, second.destinationIp);
    return sharedIp || sameNonEmpty(first.username, second.username) ||
        sameNonEmpty(first.hostname, second.hostname);
}

bool CorrelationEngine::typesRelated(EventType first, EventType second) const {
    const int firstFamily = eventFamily(first);
    return firstFamily != 0 && firstFamily == eventFamily(second);
}

std::vector<Incident> CorrelationEngine::correlate(const EventManager& manager) {
    graph_ = EntityGraph{};
    std::vector<const SecurityEvent*> ordered;
    ordered.reserve(manager.events().size());
    for (const auto& event : manager.events()) ordered.push_back(&event);
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto* first, const auto* second) {
        return first->timestamp < second->timestamp;
    });

    SlidingWindow window(config_.windowSeconds);
    for (const auto* event : ordered) {
        graph_.addEventEntities(*event);
        window.expire(event->timestamp);
        for (const auto* candidate : window.events()) {
            if (sharesEntity(*candidate, *event) && typesRelated(candidate->type, event->type)) {
                graph_.addEdge(EntityGraph::eventNode(candidate->id), EntityGraph::eventNode(event->id));
            }
        }
        window.add(*event);
    }

    std::vector<std::vector<const SecurityEvent*>> correlated;
    for (const auto& component : graph_.eventComponents()) {
        if (component.size() < config_.minimumIncidentEvents) continue;
        std::vector<const SecurityEvent*> events;
        for (const auto& id : component) {
            if (const auto* event = manager.findById(id)) events.push_back(event);
        }
        std::sort(events.begin(), events.end(), [](const auto* first, const auto* second) {
            return first->timestamp < second->timestamp;
        });
        correlated.push_back(std::move(events));
    }
    std::sort(correlated.begin(), correlated.end(), [](const auto& first, const auto& second) {
        return first.front()->timestamp < second.front()->timestamp;
    });

    std::vector<Incident> incidents;
    for (std::size_t i = 0; i < correlated.size(); ++i) {
        std::ostringstream id;
        id << "INC-" << std::setfill('0') << std::setw(4) << i + 1;
        Incident incident(id.str());
        for (const auto* event : correlated[i]) incident.addEvent(*event);
        incident.calculateRisk();
        incidents.push_back(std::move(incident));
    }
    return incidents;
}

}  // namespace soc

