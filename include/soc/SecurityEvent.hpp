#pragma once

#include <cstdint>
#include <string>

namespace soc {

enum class Severity { Informational, Low, Medium, High, Critical };

enum class EventType {
    FailedLogin,
    SuccessfulLogin,
    PrivilegeEscalation,
    FileAccess,
    ConnectionAttempt,
    PortScan,
    ServiceDiscovery,
    PrivilegedAccess,
    SensitiveFileAccess,
    OutboundConnection,
    LargeOutboundTransfer,
    NormalActivity,
    Unknown
};

struct SecurityEvent {
    std::string id;
    std::int64_t timestamp{};
    std::string sourceIp;
    std::string destinationIp;
    std::string username;
    std::string hostname;
    EventType type{EventType::Unknown};
    Severity severity{Severity::Informational};
    std::string description;
    std::string source;
    std::string scenarioLabel;
};

std::string toString(Severity severity);
std::string toString(EventType type);
Severity severityFromString(const std::string& value);
EventType eventTypeFromString(const std::string& value);
int severityWeight(Severity severity);

}  // namespace soc

