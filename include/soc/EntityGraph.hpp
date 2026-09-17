#pragma once

#include "soc/SecurityEvent.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace soc {

class EntityGraph {
public:
    void addNode(const std::string& node);
    void addEdge(const std::string& first, const std::string& second);
    void addEventEntities(const SecurityEvent& event);

    std::vector<std::string> bfs(const std::string& start) const;
    std::vector<std::string> dfs(const std::string& start) const;
    std::vector<std::vector<std::string>> eventComponents() const;

    bool hasEdge(const std::string& first, const std::string& second) const;
    std::size_t nodeCount() const noexcept { return adjacency_.size(); }
    std::size_t edgeCount() const noexcept { return edgeCount_; }

    static std::string eventNode(const std::string& eventId) { return "event:" + eventId; }

private:
    void dfsRecursive(const std::string& node,
                      std::unordered_set<std::string>& visited,
                      std::vector<std::string>& order) const;
    void eventDfsRecursive(const std::string& node,
                           std::unordered_set<std::string>& visited,
                           std::vector<std::string>& component) const;

    std::unordered_map<std::string, std::unordered_set<std::string>> adjacency_;
    std::size_t edgeCount_{0};
};

}  // namespace soc

