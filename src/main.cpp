#include "soc/CorrelationEngine.hpp"
#include "soc/EventManager.hpp"
#include "soc/IncidentPrioritizer.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace {

void printIncident(const soc::Incident& incident) {
    std::cout << "\n" << incident.id() << "\n"
              << "  Events: " << incident.eventChain().size() << "\n"
              << "  Time range: " << incident.startTimestamp() << " - "
              << incident.endTimestamp() << "\n"
              << "  IP entities: " << incident.ipAddresses().size() << "\n"
              << "  Users: " << incident.users().size() << "\n"
              << "  Hosts: " << incident.hosts().size() << "\n"
              << "  Priority: " << soc::toString(incident.priority()) << "\n"
              << "  Risk score: " << incident.riskScore() << "/100\n"
              << "  Timeline:\n";
    for (const auto& event : incident.eventChain().toVector()) {
        std::cout << "    " << event.timestamp << " | " << event.id << " | "
                  << soc::toString(event.type) << " | " << soc::toString(event.severity) << "\n";
    }
    std::cout << "  Score explanation:\n";
    for (const auto& reason : incident.explanations()) {
        std::cout << "    - " << reason << "\n";
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string csvPath = argc > 1 ? argv[1] : "data/synthetic_events.csv";
    try {
        std::cout << "Loading security events from " << csvPath << "...\n";
        soc::EventManager manager;
        const auto load = manager.loadCsv(csvPath);
        std::cout << "Events loaded: " << load.loaded << "\n"
                  << "Events rejected: " << load.rejected << "\n"
                  << "Unique source IPs: " << manager.uniqueSourceIps() << "\n"
                  << "Unique users: " << manager.uniqueUsers() << "\n"
                  << "Unique hosts: " << manager.uniqueHosts() << "\n";
        for (const auto& error : load.errors) std::cerr << "Warning: " << error << "\n";

        std::cout << "\nConstructing relationship graph and running temporal correlation...\n";
        soc::CorrelationEngine engine({300, 2});
        auto incidents = engine.correlate(manager);
        std::cout << "Graph nodes: " << engine.graph().nodeCount() << "\n"
                  << "Graph edges: " << engine.graph().edgeCount() << "\n"
                  << "Correlated incidents: " << incidents.size() << "\n";

        soc::IncidentPrioritizer prioritizer;
        for (const auto& incident : incidents) prioritizer.push(incident);
        std::cout << "\nPrioritized incident queue:\n";
        while (!prioritizer.empty()) {
            printIncident(prioritizer.top());
            prioritizer.pop();
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
}

