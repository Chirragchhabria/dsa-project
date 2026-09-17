#include "soc/EventLinkedList.hpp"

#include <utility>

namespace soc {

EventLinkedList::EventLinkedList(const EventLinkedList& other) {
    for (const auto& event : other.toVector()) append(event);
}

EventLinkedList& EventLinkedList::operator=(const EventLinkedList& other) {
    if (this == &other) return *this;
    EventLinkedList copy(other);
    *this = std::move(copy);
    return *this;
}

void EventLinkedList::append(const SecurityEvent& event) {
    auto node = std::make_unique<Node>(event);
    Node* raw = node.get();
    if (!head_) {
        head_ = std::move(node);
    } else {
        tail_->next = std::move(node);
    }
    tail_ = raw;
    ++size_;
}

void EventLinkedList::insertChronological(const SecurityEvent& event) {
    if (!head_ || event.timestamp < head_->event.timestamp) {
        auto node = std::make_unique<Node>(event);
        node->next = std::move(head_);
        head_ = std::move(node);
        if (!tail_) tail_ = head_.get();
        ++size_;
        return;
    }

    Node* current = head_.get();
    while (current->next && current->next->event.timestamp <= event.timestamp) {
        current = current->next.get();
    }
    auto node = std::make_unique<Node>(event);
    if (!current->next) tail_ = node.get();
    node->next = std::move(current->next);
    current->next = std::move(node);
    ++size_;
}

bool EventLinkedList::removeById(const std::string& id) {
    std::unique_ptr<Node>* link = &head_;
    Node* previous = nullptr;
    while (*link) {
        if ((*link)->event.id == id) {
            const bool removedTail = link->get() == tail_;
            *link = std::move((*link)->next);
            if (removedTail) tail_ = previous;
            --size_;
            return true;
        }
        previous = link->get();
        link = &((*link)->next);
    }
    return false;
}

const SecurityEvent* EventLinkedList::findById(const std::string& id) const noexcept {
    for (Node* current = head_.get(); current; current = current->next.get()) {
        if (current->event.id == id) return &current->event;
    }
    return nullptr;
}

std::vector<SecurityEvent> EventLinkedList::toVector() const {
    std::vector<SecurityEvent> result;
    result.reserve(size_);
    for (Node* current = head_.get(); current; current = current->next.get()) {
        result.push_back(current->event);
    }
    return result;
}

void EventLinkedList::clear() noexcept {
    head_.reset();
    tail_ = nullptr;
    size_ = 0;
}

}  // namespace soc

