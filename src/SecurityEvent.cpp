#include "soc/SecurityEvent.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace soc {
namespace {

std::string normalized(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return c == ' ' || c == '-' ? '_' : static_cast<char>(std::toupper(c));
    });
    return value;
}

}  // namespace

std::string toString(Severity severity) {
    switch (severity) {
        case Severity::Informational: return "INFORMATIONAL";
        case Severity::Low: return "LOW";
        case Severity::Medium: return "MEDIUM";
        case Severity::High: return "HIGH";
        case Severity::Critical: return "CRITICAL";
    }
    return "INFORMATIONAL";
}

std::string toString(EventType type) {
    switch (type) {
        case EventType::FailedLogin: return "FAILED_LOGIN";
        case EventType::SuccessfulLogin: return "SUCCESSFUL_LOGIN";
        case EventType::PrivilegeEscalation: return "PRIVILEGE_ESCALATION";
        case EventType::FileAccess: return "FILE_ACCESS";
        case EventType::ConnectionAttempt: return "CONNECTION_ATTEMPT";
        case EventType::PortScan: return "PORT_SCAN";
        case EventType::ServiceDiscovery: return "SERVICE_DISCOVERY";
        case EventType::PrivilegedAccess: return "PRIVILEGED_ACCESS";
        case EventType::SensitiveFileAccess: return "SENSITIVE_FILE_ACCESS";
        case EventType::OutboundConnection: return "OUTBOUND_CONNECTION";
        case EventType::LargeOutboundTransfer: return "LARGE_OUTBOUND_TRANSFER";
        case EventType::NormalActivity: return "NORMAL_ACTIVITY";
        case EventType::Unknown: return "UNKNOWN";
    }
    return "UNKNOWN";
}

Severity severityFromString(const std::string& value) {
    const auto key = normalized(value);
    if (key == "INFORMATIONAL" || key == "INFO") return Severity::Informational;
    if (key == "LOW") return Severity::Low;
    if (key == "MEDIUM") return Severity::Medium;
    if (key == "HIGH") return Severity::High;
    if (key == "CRITICAL") return Severity::Critical;
    throw std::invalid_argument("unknown severity: " + value);
}

EventType eventTypeFromString(const std::string& value) {
    const auto key = normalized(value);
    if (key == "FAILED_LOGIN") return EventType::FailedLogin;
    if (key == "SUCCESSFUL_LOGIN") return EventType::SuccessfulLogin;
    if (key == "PRIVILEGE_ESCALATION") return EventType::PrivilegeEscalation;
    if (key == "FILE_ACCESS") return EventType::FileAccess;
    if (key == "CONNECTION_ATTEMPT") return EventType::ConnectionAttempt;
    if (key == "PORT_SCAN") return EventType::PortScan;
    if (key == "SERVICE_DISCOVERY") return EventType::ServiceDiscovery;
    if (key == "PRIVILEGED_ACCESS") return EventType::PrivilegedAccess;
    if (key == "SENSITIVE_FILE_ACCESS") return EventType::SensitiveFileAccess;
    if (key == "OUTBOUND_CONNECTION") return EventType::OutboundConnection;
    if (key == "LARGE_OUTBOUND_TRANSFER") return EventType::LargeOutboundTransfer;
    if (key == "NORMAL_ACTIVITY") return EventType::NormalActivity;
    if (key == "UNKNOWN") return EventType::Unknown;
    throw std::invalid_argument("unknown event type: " + value);
}

int severityWeight(Severity severity) {
    switch (severity) {
        case Severity::Informational: return 1;
        case Severity::Low: return 3;
        case Severity::Medium: return 7;
        case Severity::High: return 12;
        case Severity::Critical: return 20;
    }
    return 0;
}

}  // namespace soc

