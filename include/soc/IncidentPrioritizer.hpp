#pragma once

#include "soc/Incident.hpp"

#include <cstddef>
#include <queue>
#include <vector>

namespace soc {

class IncidentPrioritizer {
public:
    void push(const Incident& incident);
    const Incident& top() const;
    void pop();
    bool empty() const noexcept { return queue_.empty(); }
    std::size_t size() const noexcept { return queue_.size(); }

private:
    struct LowerPriority {
        bool operator()(const Incident* first, const Incident* second) const;
    };
    std::priority_queue<const Incident*, std::vector<const Incident*>, LowerPriority> queue_;
};

}  // namespace soc

