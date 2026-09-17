#include "soc/CorrelationEngine.hpp"
#include "soc/EntityGraph.hpp"
#include "soc/EventLinkedList.hpp"
#include "soc/EventManager.hpp"
#include "soc/IncidentPrioritizer.hpp"
#include "soc/SlidingWindow.hpp"

#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int passed = 0;
int failed = 0;

#define CHECK(condition) do { \
    if (!(condition)) throw std::runtime_error(std::string("check failed: ") + #condition); \
} while (false)

void run(const std::string& name, const std::function<void()>& test) {
    try {
        test();
        ++passed;
        std::cout << "[PASS] " << name << "\n";
    } catch (const std::exception& error) {
        ++failed;
        std::cerr << "[FAIL] " << name << ": " << error.what() << "\n";
    }
}

soc::SecurityEvent event(std::string id, std::int64_t timestamp,
                         soc::EventType type = soc::EventType::FailedLogin,
                         soc::Severity severity = soc::Severity::Medium) {
    soc::SecurityEvent result;
    result.id = std::move(id);
    result.timestamp = timestamp;
    result.sourceIp = "10.0.0.8";
    result.destinationIp = "10.0.0.20";
    result.username = "alex";
    result.hostname = "workstation-8";
    result.type = type;
    result.severity = severity;
    return result;
}

void testCsvParsingAndMalformedRows() {
    const std::string path = "/tmp/soc_engine_csv_test.csv";
    {
        std::ofstream csv(path);
        csv << "event_id,timestamp,source_ip,destination_ip,username,hostname,event_type,severity,description,source,scenario_label\n"
            << "E1,100,10.0.0.1,10.0.0.2,alex,host-a,FAILED_LOGIN,MEDIUM,\"failed, interactive login\",auth,test\n"
            << "E2,not-a-time,10.0.0.1,10.0.0.2,alex,host-a,FAILED_LOGIN,MEDIUM,bad,auth,test\n"
            << "E1,110,10.0.0.1,10.0.0.2,alex,host-a,SUCCESSFUL_LOGIN,HIGH,duplicate,auth,test\n";
    }
    soc::EventManager manager;
    const auto result = manager.loadCsv(path);
    std::remove(path.c_str());
    CHECK(result.loaded == 1);
    CHECK(result.rejected == 2);
    CHECK(manager.events().front().description == "failed, interactive login");
    CHECK(manager.findById("E1") != nullptr);
}

void testHashIndexes() {
    soc::EventManager manager;
    manager.addEvent(event("E1", 100));
    auto second = event("E2", 110);
    second.sourceIp = "10.0.0.9";
    manager.addEvent(second);
    CHECK(manager.bySourceIp("10.0.0.8").size() == 1);
    CHECK(manager.byUser("alex").size() == 2);
    CHECK(manager.byHost("workstation-8").size() == 2);
    CHECK(manager.uniqueSourceIps() == 2);
}

void testLinkedListOperations() {
    soc::EventLinkedList list;
    list.insertChronological(event("E2", 200));
    list.insertChronological(event("E1", 100));
    list.insertChronological(event("E3", 300));
    const auto ordered = list.toVector();
    CHECK(ordered.size() == 3);
    CHECK(ordered[0].id == "E1" && ordered[2].id == "E3");
    CHECK(list.findById("E2") != nullptr);
    CHECK(list.removeById("E3"));
    CHECK(list.size() == 2);
    soc::EventLinkedList copy = list;
    list.clear();
    CHECK(copy.size() == 2 && list.empty());
}

void testSlidingWindowExpiration() {
    const auto first = event("E1", 100);
    const auto second = event("E2", 150);
    const auto third = event("E3", 221);
    soc::SlidingWindow window(120);
    window.add(first);
    window.add(second);
    window.add(third);
    CHECK(window.size() == 2);
    CHECK(window.events().front()->id == "E2");
}

void testGraphBfsAndRecursiveDfs() {
    soc::EntityGraph graph;
    graph.addEdge("event:E1", "event:E2");
    graph.addEdge("event:E2", "event:E3");
    graph.addEdge("event:E1", "user:alex");
    CHECK(graph.nodeCount() == 4);
    CHECK(graph.edgeCount() == 3);
    CHECK(graph.hasEdge("event:E2", "event:E1"));
    CHECK(graph.bfs("event:E1").size() == 4);
    CHECK(graph.dfs("event:E1").size() == 4);
    const auto components = graph.eventComponents();
    CHECK(components.size() == 1 && components.front().size() == 3);
}

void testCorrelationRulesAndIncidentCreation() {
    soc::EventManager manager;
    manager.addEvent(event("E1", 100, soc::EventType::FailedLogin));
    manager.addEvent(event("E2", 130, soc::EventType::SuccessfulLogin, soc::Severity::High));
    manager.addEvent(event("E3", 160, soc::EventType::PrivilegeEscalation, soc::Severity::Critical));
    auto unrelated = event("E4", 170, soc::EventType::PortScan);
    unrelated.sourceIp = "192.0.2.55";
    unrelated.destinationIp = "192.0.2.80";
    unrelated.username.clear();
    unrelated.hostname = "scanner";
    manager.addEvent(unrelated);
    auto late = event("E5", 1000, soc::EventType::FileAccess);
    manager.addEvent(late);

    soc::CorrelationEngine engine({300, 2});
    const auto incidents = engine.correlate(manager);
    CHECK(incidents.size() == 1);
    CHECK(incidents.front().eventChain().size() == 3);
    CHECK(incidents.front().id() == "INC-0001");
    CHECK(incidents.front().riskScore() >= 50);
    CHECK(!engine.graph().hasEdge("event:E3", "event:E4"));
    CHECK(!engine.graph().hasEdge("event:E3", "event:E5"));
}

void testPriorityOrdering() {
    soc::Incident lower("LOWER");
    lower.addEvent(event("L1", 100, soc::EventType::FailedLogin, soc::Severity::Low));
    lower.addEvent(event("L2", 110, soc::EventType::SuccessfulLogin, soc::Severity::Low));
    lower.calculateRisk();

    soc::Incident higher("HIGHER");
    higher.addEvent(event("H1", 200, soc::EventType::PrivilegedAccess, soc::Severity::Critical));
    higher.addEvent(event("H2", 210, soc::EventType::SensitiveFileAccess, soc::Severity::High));
    higher.addEvent(event("H3", 220, soc::EventType::LargeOutboundTransfer, soc::Severity::Critical));
    higher.calculateRisk();

    std::vector<soc::Incident> incidents;
    incidents.push_back(lower);
    incidents.push_back(higher);
    soc::IncidentPrioritizer queue;
    for (const auto& incident : incidents) queue.push(incident);
    CHECK(queue.top().id() == "HIGHER");
    queue.pop();
    CHECK(queue.top().id() == "LOWER");
}

}  // namespace

int main() {
    run("CSV parsing and malformed row handling", testCsvParsingAndMalformedRows);
    run("hash-map indexes", testHashIndexes);
    run("custom linked-list operations", testLinkedListOperations);
    run("sliding-window expiration", testSlidingWindowExpiration);
    run("graph edges, BFS, and recursive DFS", testGraphBfsAndRecursiveDfs);
    run("correlation rules and incident creation", testCorrelationRulesAndIncidentCreation);
    run("priority-queue ordering", testPriorityOrdering);
    std::cout << "\nTests passed: " << passed << ", failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

