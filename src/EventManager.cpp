#include "soc/EventManager.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace soc {
namespace {

std::vector<std::string> parseCsvRow(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') {
                field.push_back('"');
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (c == ',' && !quoted) {
            fields.push_back(field);
            field.clear();
        } else {
            field.push_back(c);
        }
    }
    if (quoted) throw std::invalid_argument("unclosed quoted field");
    fields.push_back(field);
    return fields;
}

std::string required(const std::vector<std::string>& row,
                     const std::unordered_map<std::string, std::size_t>& columns,
                     const std::string& name) {
    const auto it = columns.find(name);
    if (it == columns.end() || it->second >= row.size() || row[it->second].empty()) {
        throw std::invalid_argument("missing required field '" + name + "'");
    }
    return row[it->second];
}

std::string optional(const std::vector<std::string>& row,
                     const std::unordered_map<std::string, std::size_t>& columns,
                     const std::string& name) {
    const auto it = columns.find(name);
    return it == columns.end() || it->second >= row.size() ? "" : row[it->second];
}

}  // namespace

LoadResult EventManager::loadCsv(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open CSV file: " + path);

    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("CSV file is empty: " + path);
    const auto header = parseCsvRow(line);
    std::unordered_map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < header.size(); ++i) columns[header[i]] = i;

    for (const auto* name : {"event_id", "timestamp", "event_type", "severity"}) {
        if (columns.count(name) == 0) throw std::runtime_error("CSV missing column: " + std::string(name));
    }

    LoadResult result;
    std::size_t lineNumber = 1;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (line.empty()) continue;
        try {
            const auto row = parseCsvRow(line);
            SecurityEvent event;
            event.id = required(row, columns, "event_id");
            const auto timestampText = required(row, columns, "timestamp");
            std::size_t parsed = 0;
            event.timestamp = std::stoll(timestampText, &parsed);
            if (parsed != timestampText.size() || event.timestamp < 0) {
                throw std::invalid_argument("timestamp must be a non-negative integer");
            }
            event.sourceIp = optional(row, columns, "source_ip");
            event.destinationIp = optional(row, columns, "destination_ip");
            event.username = optional(row, columns, "username");
            event.hostname = optional(row, columns, "hostname");
            event.type = eventTypeFromString(required(row, columns, "event_type"));
            event.severity = severityFromString(required(row, columns, "severity"));
            event.description = optional(row, columns, "description");
            event.source = optional(row, columns, "source");
            event.scenarioLabel = optional(row, columns, "scenario_label");
            addEvent(std::move(event));
            ++result.loaded;
        } catch (const std::exception& error) {
            ++result.rejected;
            result.errors.push_back("line " + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    return result;
}

void EventManager::addEvent(SecurityEvent event) {
    if (event.id.empty()) throw std::invalid_argument("event ID cannot be empty");
    if (event.timestamp < 0) throw std::invalid_argument("event timestamp cannot be negative");
    if (idIndex_.count(event.id) != 0) throw std::invalid_argument("duplicate event ID: " + event.id);
    if (event.sourceIp.empty() && event.destinationIp.empty() && event.username.empty() && event.hostname.empty()) {
        throw std::invalid_argument("event must contain at least one correlatable entity");
    }
    events_.push_back(std::move(event));
    indexEvent(events_.size() - 1);
}

void EventManager::indexEvent(std::size_t position) {
    const auto& event = events_[position];
    idIndex_[event.id] = position;
    if (!event.sourceIp.empty()) sourceIpIndex_[event.sourceIp].push_back(position);
    if (!event.destinationIp.empty()) destinationIpIndex_[event.destinationIp].push_back(position);
    if (!event.username.empty()) userIndex_[event.username].push_back(position);
    if (!event.hostname.empty()) hostIndex_[event.hostname].push_back(position);
}

const SecurityEvent* EventManager::findById(const std::string& id) const {
    const auto it = idIndex_.find(id);
    return it == idIndex_.end() ? nullptr : &events_[it->second];
}

std::vector<const SecurityEvent*> EventManager::lookup(const Index& index, const std::string& value) const {
    std::vector<const SecurityEvent*> result;
    const auto it = index.find(value);
    if (it == index.end()) return result;
    result.reserve(it->second.size());
    for (const auto position : it->second) result.push_back(&events_[position]);
    return result;
}

std::vector<const SecurityEvent*> EventManager::bySourceIp(const std::string& value) const {
    return lookup(sourceIpIndex_, value);
}
std::vector<const SecurityEvent*> EventManager::byDestinationIp(const std::string& value) const {
    return lookup(destinationIpIndex_, value);
}
std::vector<const SecurityEvent*> EventManager::byUser(const std::string& value) const {
    return lookup(userIndex_, value);
}
std::vector<const SecurityEvent*> EventManager::byHost(const std::string& value) const {
    return lookup(hostIndex_, value);
}

}  // namespace soc
