#include "soc/EntityGraph.hpp"

#include <algorithm>
#include <queue>

namespace soc {

void EntityGraph::addNode(const std::string& node) {
    if (!node.empty()) adjacency_.try_emplace(node);
}

void EntityGraph::addEdge(const std::string& first, const std::string& second) {
    if (first.empty() || second.empty() || first == second) return;
    addNode(first);
    addNode(second);
    if (adjacency_[first].insert(second).second) {
        adjacency_[second].insert(first);
        ++edgeCount_;
    }
}

void EntityGraph::addEventEntities(const SecurityEvent& event) {
    const auto eventId = eventNode(event.id);
    addNode(eventId);
    if (!event.sourceIp.empty()) addEdge(eventId, "ip:" + event.sourceIp);
    if (!event.destinationIp.empty()) addEdge(eventId, "ip:" + event.destinationIp);
    if (!event.username.empty()) addEdge(eventId, "user:" + event.username);
    if (!event.hostname.empty()) addEdge(eventId, "host:" + event.hostname);
}

std::vector<std::string> EntityGraph::bfs(const std::string& start) const {
    if (adjacency_.count(start) == 0) return {};
    std::vector<std::string> order;
    std::unordered_set<std::string> visited{start};
    std::queue<std::string> pending;
    pending.push(start);
    while (!pending.empty()) {
        auto node = pending.front();
        pending.pop();
        order.push_back(node);
        std::vector<std::string> neighbors(adjacency_.at(node).begin(), adjacency_.at(node).end());
        std::sort(neighbors.begin(), neighbors.end());
        for (const auto& neighbor : neighbors) {
            if (visited.insert(neighbor).second) pending.push(neighbor);
        }
    }
    return order;
}

void EntityGraph::dfsRecursive(const std::string& node,
                               std::unordered_set<std::string>& visited,
                               std::vector<std::string>& order) const {
    visited.insert(node);
    order.push_back(node);
    std::vector<std::string> neighbors(adjacency_.at(node).begin(), adjacency_.at(node).end());
    std::sort(neighbors.begin(), neighbors.end());
    for (const auto& neighbor : neighbors) {
        if (visited.count(neighbor) == 0) dfsRecursive(neighbor, visited, order);
    }
}

std::vector<std::string> EntityGraph::dfs(const std::string& start) const {
    if (adjacency_.count(start) == 0) return {};
    std::vector<std::string> order;
    std::unordered_set<std::string> visited;
    dfsRecursive(start, visited, order);
    return order;
}

void EntityGraph::eventDfsRecursive(const std::string& node,
                                    std::unordered_set<std::string>& visited,
                                    std::vector<std::string>& component) const {
    visited.insert(node);
    component.push_back(node.substr(6));
    for (const auto& neighbor : adjacency_.at(node)) {
        if (neighbor.rfind("event:", 0) == 0 && visited.count(neighbor) == 0) {
            eventDfsRecursive(neighbor, visited, component);
        }
    }
}

std::vector<std::vector<std::string>> EntityGraph::eventComponents() const {
    std::vector<std::string> eventNodes;
    for (const auto& [node, ignored] : adjacency_) {
        (void)ignored;
        if (node.rfind("event:", 0) == 0) eventNodes.push_back(node);
    }
    std::sort(eventNodes.begin(), eventNodes.end());

    std::vector<std::vector<std::string>> components;
    std::unordered_set<std::string> visited;
    for (const auto& node : eventNodes) {
        if (visited.count(node) != 0) continue;
        std::vector<std::string> component;
        eventDfsRecursive(node, visited, component);
        std::sort(component.begin(), component.end());
        components.push_back(std::move(component));
    }
    return components;
}

bool EntityGraph::hasEdge(const std::string& first, const std::string& second) const {
    const auto it = adjacency_.find(first);
    return it != adjacency_.end() && it->second.count(second) != 0;
}

}  // namespace soc
