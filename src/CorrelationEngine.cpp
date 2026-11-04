#include "soc/CorrelationEngine.hpp"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
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
        case EventType::DenialOfService:
            return 4;
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

std::vector<std::string> entityKeys(const SecurityEvent& event) {
    std::vector<std::string> keys;
    if (!event.sourceIp.empty()) keys.push_back("ip:" + event.sourceIp);
    if (!event.destinationIp.empty() && event.destinationIp != event.sourceIp) {
        keys.push_back("ip:" + event.destinationIp);
    }
    if (!event.username.empty()) keys.push_back("user:" + event.username);
    if (!event.hostname.empty()) keys.push_back("host:" + event.hostname);
    return keys;
}

}  // namespace

CorrelationEngine::CorrelationEngine(CorrelationConfig config) : config_(config) {
    if (config_.windowSeconds <= 0) throw std::invalid_argument("correlation window must be positive");
    if (config_.minimumIncidentEvents < 2) {
        throw std::invalid_argument("minimum incident size must be at least two");
    }
    if (config_.enabledFamilies == 0 || (config_.enabledFamilies & ~15u) != 0) {
        throw std::invalid_argument("enabled families must be a nonempty subset of the four known families");
    }
}

bool CorrelationEngine::sharesEntity(const SecurityEvent& first, const SecurityEvent& second) const {
    const auto other = entityKeys(second);
    for (const auto& key : entityKeys(first)) {
        if (std::find(other.begin(), other.end(), key) != other.end()) return true;
    }
    return false;
}

bool CorrelationEngine::typesRelated(EventType first, EventType second) const {
    const int firstFamily = eventFamily(first);
    return firstFamily != 0 && firstFamily == eventFamily(second) &&
        (config_.enabledFamilies & (1u << (firstFamily - 1))) != 0;
}

PairDecision CorrelationEngine::explainPair(const SecurityEvent& first,
                                            const SecurityEvent& second) const {
    const auto distance = first.timestamp > second.timestamp ?
        first.timestamp - second.timestamp : second.timestamp - first.timestamp;
    return {distance <= config_.windowSeconds, sharesEntity(first, second),
            typesRelated(first.type, second.type)};
}

std::vector<Incident> CorrelationEngine::correlate(const EventManager& manager) {
    graph_ = EntityGraph{};
    std::vector<const SecurityEvent*> ordered;
    ordered.reserve(manager.events().size());
    for (const auto& event : manager.events()) ordered.push_back(&event);
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto* first, const auto* second) {
        return first->timestamp < second->timestamp;
    });

    std::unordered_map<std::string, std::deque<const SecurityEvent*>> recentByEntity;
    std::deque<const SecurityEvent*> active;
    for (const auto* event : ordered) {
        graph_.addEventEntities(*event);
        while (!active.empty() && event->timestamp - active.front()->timestamp > config_.windowSeconds) {
            for (const auto& key : entityKeys(*active.front())) {
                auto bucket = recentByEntity.find(key);
                bucket->second.pop_front();
                if (bucket->second.empty()) recentByEntity.erase(bucket);
            }
            active.pop_front();
        }
        const auto keys = entityKeys(*event);
        std::unordered_set<const SecurityEvent*> candidates;
        for (const auto& key : keys) {
            const auto bucket = recentByEntity.find(key);
            if (bucket != recentByEntity.end()) {
                candidates.insert(bucket->second.begin(), bucket->second.end());
            }
        }
        for (const auto* candidate : candidates) {
            if (typesRelated(candidate->type, event->type)) {
                graph_.addEdge(EntityGraph::eventNode(candidate->id), EntityGraph::eventNode(event->id));
            }
        }
        for (const auto& key : keys) recentByEntity[key].push_back(event);
        active.push_back(event);
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

