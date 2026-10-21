#include "soc/CorrelationEngine.hpp"
#include "soc/EventManager.hpp"
#include "soc/IncidentPrioritizer.hpp"

#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string path{"data/synthetic_events.csv"};
    bool json{false};
    std::int64_t window{300};
    std::size_t minimum{2};
};

void usage(std::ostream& out) {
    out << "Usage: soc_cli [CSV_PATH] [--format text|json] [--window SECONDS] [--min-events COUNT]\n"
        << "       soc_cli --help\n";
}

long long positiveNumber(const std::string& value, const std::string& option) {
    std::size_t parsed = 0;
    long long number = 0;
    try { number = std::stoll(value, &parsed); }
    catch (const std::exception&) { throw std::invalid_argument(option + " requires a positive integer"); }
    if (parsed != value.size() || number <= 0) throw std::invalid_argument(option + " requires a positive integer");
    return number;
}

Options parseOptions(int argc, char* argv[]) {
    Options options;
    bool hasPath = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") { usage(std::cout); std::exit(EXIT_SUCCESS); }
        if (arg == "--format" || arg == "--window" || arg == "--min-events") {
            if (++i >= argc) throw std::invalid_argument("missing value for " + arg);
            const std::string value = argv[i];
            if (arg == "--format") {
                if (value != "text" && value != "json") throw std::invalid_argument("--format must be text or json");
                options.json = value == "json";
            } else if (arg == "--window") {
                options.window = positiveNumber(value, arg);
            } else {
                options.minimum = positiveNumber(value, arg);
                if (options.minimum < 2) throw std::invalid_argument("--min-events must be at least 2");
            }
        } else if (!arg.empty() && arg.front() == '-') {
            throw std::invalid_argument("unknown option: " + arg);
        } else if (!hasPath) {
            options.path = arg;
            hasPath = true;
        } else {
            throw std::invalid_argument("only one CSV path is allowed");
        }
    }
    return options;
}

std::string utcTime(std::int64_t timestamp) {
    const auto seconds = static_cast<std::time_t>(timestamp);
    const std::tm* utc = std::gmtime(&seconds);
    if (!utc) return std::to_string(timestamp);
    std::ostringstream formatted;
    formatted << std::put_time(utc, "%Y-%m-%d %H:%M:%S UTC");
    return formatted.str();
}

std::string jsonString(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
                else out << static_cast<char>(c);
        }
    }
    out << '"';
    return out.str();
}

void printIncident(const soc::Incident& incident) {
    std::cout << "\n" << incident.id() << "\n"
              << "  Events: " << incident.eventChain().size() << "\n"
              << "  Time range: " << utcTime(incident.startTimestamp()) << " to "
              << utcTime(incident.endTimestamp()) << "\n"
              << "  IP entities: " << incident.ipAddresses().size() << "\n"
              << "  Users: " << incident.users().size() << "\n"
              << "  Hosts: " << incident.hosts().size() << "\n"
              << "  Priority: " << soc::toString(incident.priority()) << "\n"
              << "  Risk score: " << incident.riskScore() << "/100\n"
              << "  Timeline:\n";
    for (const auto& event : incident.eventChain().toVector()) {
        std::cout << "    " << utcTime(event.timestamp) << " | " << event.id << " | "
                  << soc::toString(event.type) << " | " << soc::toString(event.severity) << "\n";
        if (!event.description.empty()) std::cout << "      " << event.description << "\n";
    }
    std::cout << "  Score explanation:\n";
    for (const auto& reason : incident.explanations()) std::cout << "    - " << reason << "\n";
}

void printJson(const Options& options, const soc::LoadResult& load,
               const soc::EventManager& manager, const soc::CorrelationEngine& engine,
               const std::vector<soc::Incident>& incidents) {
    std::cout << "{\"schema_version\":1,\"input\":" << jsonString(options.path)
              << ",\"config\":{\"window_seconds\":" << options.window
              << ",\"minimum_incident_events\":" << options.minimum << "}"
              << ",\"summary\":{\"events_loaded\":" << load.loaded
              << ",\"events_rejected\":" << load.rejected
              << ",\"unique_source_ips\":" << manager.uniqueSourceIps()
              << ",\"unique_users\":" << manager.uniqueUsers()
              << ",\"unique_hosts\":" << manager.uniqueHosts()
              << ",\"graph_nodes\":" << engine.graph().nodeCount()
              << ",\"graph_edges\":" << engine.graph().edgeCount()
              << ",\"incidents\":" << incidents.size() << "},\"errors\":[";
    for (std::size_t i = 0; i < load.errors.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << jsonString(load.errors[i]);
    }
    std::cout << "],\"incidents\":[";
    soc::IncidentPrioritizer prioritizer;
    for (const auto& incident : incidents) prioritizer.push(incident);
    for (std::size_t i = 0; i < incidents.size(); ++i) {
        if (i) std::cout << ',';
        const auto& incident = prioritizer.top();
        std::cout << "{\"id\":" << jsonString(incident.id())
                  << ",\"risk_score\":" << incident.riskScore()
                  << ",\"priority\":" << jsonString(soc::toString(incident.priority()))
                  << ",\"start_timestamp\":" << incident.startTimestamp()
                  << ",\"end_timestamp\":" << incident.endTimestamp() << ",\"events\":[";
        const auto events = incident.eventChain().toVector();
        for (std::size_t j = 0; j < events.size(); ++j) {
            if (j) std::cout << ',';
            const auto& event = events[j];
            std::cout << "{\"id\":" << jsonString(event.id)
                      << ",\"timestamp\":" << event.timestamp
                      << ",\"type\":" << jsonString(soc::toString(event.type))
                      << ",\"severity\":" << jsonString(soc::toString(event.severity))
                      << ",\"description\":" << jsonString(event.description) << '}';
        }
        std::cout << "],\"explanations\":[";
        for (std::size_t j = 0; j < incident.explanations().size(); ++j) {
            if (j) std::cout << ',';
            std::cout << jsonString(incident.explanations()[j]);
        }
        std::cout << "]}";
        prioritizer.pop();
    }
    std::cout << "]}\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        const Options options = parseOptions(argc, argv);
        soc::EventManager manager;
        const auto load = manager.loadCsv(options.path);
        soc::CorrelationEngine engine({options.window, options.minimum});
        auto incidents = engine.correlate(manager);
        if (options.json) {
            printJson(options, load, manager, engine, incidents);
            return EXIT_SUCCESS;
        }
        std::cout << "Loading security events from " << options.path << "...\n"
                  << "Events loaded: " << load.loaded << "\n"
                  << "Events rejected: " << load.rejected << "\n"
                  << "Unique source IPs: " << manager.uniqueSourceIps() << "\n"
                  << "Unique users: " << manager.uniqueUsers() << "\n"
                  << "Unique hosts: " << manager.uniqueHosts() << "\n";
        for (const auto& error : load.errors) std::cerr << "Warning: " << error << "\n";
        std::cout << "\nConstructing relationship graph and running temporal correlation...\n"
                  << "Graph nodes: " << engine.graph().nodeCount() << "\n"
                  << "Graph edges: " << engine.graph().edgeCount() << "\n"
                  << "Correlated incidents: " << incidents.size() << "\n";
        soc::IncidentPrioritizer prioritizer;
        for (const auto& incident : incidents) prioritizer.push(incident);
        std::cout << "\nPrioritized incident queue:\n";
        while (!prioritizer.empty()) { printIncident(prioritizer.top()); prioritizer.pop(); }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << "\n";
        usage(std::cerr);
        return EXIT_FAILURE;
    }
}
