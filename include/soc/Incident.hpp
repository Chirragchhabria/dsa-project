#pragma once

#include "soc/EventLinkedList.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace soc {

class Incident {
public:
    explicit Incident(std::string id);

    void addEvent(const SecurityEvent& event);
    void calculateRisk();

    const std::string& id() const noexcept { return id_; }
    const EventLinkedList& eventChain() const noexcept { return eventChain_; }
    const std::set<std::string>& ipAddresses() const noexcept { return ipAddresses_; }
    const std::set<std::string>& users() const noexcept { return users_; }
    const std::set<std::string>& hosts() const noexcept { return hosts_; }
    std::int64_t startTimestamp() const noexcept { return startTimestamp_; }
    std::int64_t endTimestamp() const noexcept { return endTimestamp_; }
    int riskScore() const noexcept { return riskScore_; }
    Severity priority() const noexcept { return priority_; }
    const std::vector<std::string>& explanations() const noexcept { return explanations_; }

private:
    std::string id_;
    EventLinkedList eventChain_;
    std::set<std::string> ipAddresses_;
    std::set<std::string> users_;
    std::set<std::string> hosts_;
    std::int64_t startTimestamp_{0};
    std::int64_t endTimestamp_{0};
    int riskScore_{0};
    Severity priority_{Severity::Informational};
    std::vector<std::string> explanations_;
};

}  // namespace soc

