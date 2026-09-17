#pragma once

#include "soc/SecurityEvent.hpp"

#include <cstdint>
#include <deque>
#include <vector>

namespace soc {

class SlidingWindow {
public:
    explicit SlidingWindow(std::int64_t durationSeconds);

    void add(const SecurityEvent& event);
    void expire(std::int64_t currentTimestamp);
    std::vector<const SecurityEvent*> events() const;
    std::size_t size() const noexcept { return events_.size(); }
    std::int64_t durationSeconds() const noexcept { return durationSeconds_; }

private:
    std::int64_t durationSeconds_;
    std::deque<const SecurityEvent*> events_;
};

}  // namespace soc

