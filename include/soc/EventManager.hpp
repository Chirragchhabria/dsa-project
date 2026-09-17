#pragma once

#include "soc/SecurityEvent.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace soc {

struct LoadResult {
    std::size_t loaded{};
    std::size_t rejected{};
    std::vector<std::string> errors;
};

class EventManager {
public:
    LoadResult loadCsv(const std::string& path);
    void addEvent(SecurityEvent event);

    const std::vector<SecurityEvent>& events() const noexcept { return events_; }
    const SecurityEvent* findById(const std::string& id) const;
    std::vector<const SecurityEvent*> bySourceIp(const std::string& value) const;
    std::vector<const SecurityEvent*> byDestinationIp(const std::string& value) const;
    std::vector<const SecurityEvent*> byUser(const std::string& value) const;
    std::vector<const SecurityEvent*> byHost(const std::string& value) const;

    std::size_t uniqueSourceIps() const noexcept { return sourceIpIndex_.size(); }
    std::size_t uniqueUsers() const noexcept { return userIndex_.size(); }
    std::size_t uniqueHosts() const noexcept { return hostIndex_.size(); }

private:
    using Index = std::unordered_map<std::string, std::vector<std::size_t>>;
    std::vector<const SecurityEvent*> lookup(const Index& index, const std::string& value) const;
    void indexEvent(std::size_t position);

    std::vector<SecurityEvent> events_;
    std::unordered_map<std::string, std::size_t> idIndex_;
    Index sourceIpIndex_;
    Index destinationIpIndex_;
    Index userIndex_;
    Index hostIndex_;
};

}  // namespace soc

