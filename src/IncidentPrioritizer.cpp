#include "soc/IncidentPrioritizer.hpp"

#include <stdexcept>

namespace soc {

bool IncidentPrioritizer::LowerPriority::operator()(const Incident* first, const Incident* second) const {
    if (first->riskScore() != second->riskScore()) return first->riskScore() < second->riskScore();
    if (first->startTimestamp() != second->startTimestamp()) {
        return first->startTimestamp() > second->startTimestamp();
    }
    return first->id() > second->id();
}

void IncidentPrioritizer::push(const Incident& incident) {
    queue_.push(&incident);
}

const Incident& IncidentPrioritizer::top() const {
    if (queue_.empty()) throw std::out_of_range("incident priority queue is empty");
    return *queue_.top();
}

void IncidentPrioritizer::pop() {
    if (queue_.empty()) throw std::out_of_range("incident priority queue is empty");
    queue_.pop();
}

}  // namespace soc
