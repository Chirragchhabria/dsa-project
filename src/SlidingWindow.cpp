#include "soc/SlidingWindow.hpp"

#include <stdexcept>

namespace soc {

SlidingWindow::SlidingWindow(std::int64_t durationSeconds) : durationSeconds_(durationSeconds) {
    if (durationSeconds <= 0) throw std::invalid_argument("window duration must be positive");
}

void SlidingWindow::add(const SecurityEvent& event) {
    if (!events_.empty() && event.timestamp < events_.back()->timestamp) {
        throw std::invalid_argument("sliding window events must arrive in timestamp order");
    }
    expire(event.timestamp);
    events_.push_back(&event);
}

void SlidingWindow::expire(std::int64_t currentTimestamp) {
    while (!events_.empty() && currentTimestamp - events_.front()->timestamp > durationSeconds_) {
        events_.pop_front();
    }
}

std::vector<const SecurityEvent*> SlidingWindow::events() const {
    return {events_.begin(), events_.end()};
}

}  // namespace soc

