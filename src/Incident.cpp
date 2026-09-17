#include "soc/Incident.hpp"

#include <algorithm>
#include <stdexcept>

namespace soc {

Incident::Incident(std::string id) : id_(std::move(id)) {
    if (id_.empty()) throw std::invalid_argument("incident ID cannot be empty");
}

void Incident::addEvent(const SecurityEvent& event) {
    if (eventChain_.findById(event.id)) return;
    eventChain_.insertChronological(event);
    const auto events = eventChain_.toVector();
    startTimestamp_ = events.front().timestamp;
    endTimestamp_ = events.back().timestamp;
    if (!event.sourceIp.empty()) ipAddresses_.insert(event.sourceIp);
    if (!event.destinationIp.empty()) ipAddresses_.insert(event.destinationIp);
    if (!event.username.empty()) users_.insert(event.username);
    if (!event.hostname.empty()) hosts_.insert(event.hostname);
}

void Incident::calculateRisk() {
    explanations_.clear();
    const auto events = eventChain_.toVector();
    int severityTotal = 0;
    bool hasPrivilegeActivity = false;
    bool hasExfiltrationActivity = false;
    for (const auto& event : events) {
        severityTotal += severityWeight(event.severity);
        hasPrivilegeActivity = hasPrivilegeActivity ||
            event.type == EventType::PrivilegeEscalation || event.type == EventType::PrivilegedAccess;
        hasExfiltrationActivity = hasExfiltrationActivity ||
            event.type == EventType::SensitiveFileAccess || event.type == EventType::LargeOutboundTransfer;
    }

    riskScore_ = severityTotal;
    explanations_.push_back("severity contribution: " + std::to_string(severityTotal));
    if (events.size() > 1) {
        const int sizeBonus = std::min(20, static_cast<int>((events.size() - 1) * 2));
        riskScore_ += sizeBonus;
        explanations_.push_back("multi-event chain bonus: " + std::to_string(sizeBonus));
    }
    if (hasPrivilegeActivity) {
        riskScore_ += 15;
        explanations_.push_back("privilege-related activity: 15");
    }
    if (hasExfiltrationActivity) {
        riskScore_ += 20;
        explanations_.push_back("sensitive access or large transfer: 20");
    }
    if (hosts_.size() > 1) {
        const int hostBonus = std::min(12, static_cast<int>((hosts_.size() - 1) * 3));
        riskScore_ += hostBonus;
        explanations_.push_back("affected-host diversity: " + std::to_string(hostBonus));
    }
    if (events.size() >= 3 && endTimestamp_ - startTimestamp_ <= 300) {
        riskScore_ += 10;
        explanations_.push_back("dense activity within five minutes: 10");
    }
    if (riskScore_ > 100) {
        explanations_.push_back("score capped at 100");
        riskScore_ = 100;
    }

    if (riskScore_ >= 75) priority_ = Severity::Critical;
    else if (riskScore_ >= 50) priority_ = Severity::High;
    else if (riskScore_ >= 25) priority_ = Severity::Medium;
    else if (riskScore_ >= 10) priority_ = Severity::Low;
    else priority_ = Severity::Informational;
}

}  // namespace soc
