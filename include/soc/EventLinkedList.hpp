#pragma once

#include "soc/SecurityEvent.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace soc {

class EventLinkedList {
public:
    EventLinkedList() = default;
    EventLinkedList(const EventLinkedList& other);
    EventLinkedList& operator=(const EventLinkedList& other);
    EventLinkedList(EventLinkedList&&) noexcept = default;
    EventLinkedList& operator=(EventLinkedList&&) noexcept = default;
    ~EventLinkedList() = default;

    void append(const SecurityEvent& event);
    void insertChronological(const SecurityEvent& event);
    bool removeById(const std::string& id);
    const SecurityEvent* findById(const std::string& id) const noexcept;
    std::vector<SecurityEvent> toVector() const;
    void clear() noexcept;

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

private:
    struct Node {
        explicit Node(const SecurityEvent& value) : event(value) {}
        SecurityEvent event;
        std::unique_ptr<Node> next;
    };

    std::unique_ptr<Node> head_;
    Node* tail_{nullptr};
    std::size_t size_{0};
};

}  // namespace soc

