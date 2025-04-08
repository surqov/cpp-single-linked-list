#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

template <typename Type>
class SingleLinkedList {
  struct Node {
    Type value_;
    Node *next_node_ = nullptr;
    Node() = default;
    Node(const Type &value, Node *node) : value_(value), next_node_(node) {}
    explicit operator bool() const noexcept { return next_node_ != nullptr; }
  };

  template <typename ValueType>
  class BasicIterator {
    friend class SingleLinkedList;
    Node *node_ = nullptr;
    explicit BasicIterator(Node *node) : node_(node) {}

   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Type;
    using difference_type = std::ptrdiff_t;
    using pointer = ValueType *;
    using reference = ValueType &;

    BasicIterator() = default;

    BasicIterator &operator=(const BasicIterator &rhs) = default;

    BasicIterator(const BasicIterator<Type> &other) noexcept
    : node_(other.node_)
    {}

    [[nodiscard]] bool operator==(
        const BasicIterator<const Type> &rhs) const noexcept {
      return node_ == rhs.node_;
    }

    [[nodiscard]] bool operator!=(
        const BasicIterator<const Type> &rhs) const noexcept {
      return !(*this == rhs);
    }
    [[nodiscard]] bool operator==(
        const BasicIterator<Type> &rhs) const noexcept {
      return node_ == rhs.node_;
    }

    [[nodiscard]] bool operator!=(
        const BasicIterator<Type> &rhs) const noexcept {
      return !(*this == rhs);
    }

    BasicIterator &operator++() noexcept {
      assert(node_);
      node_ = node_->next_node_;
      return *this;
    }

    BasicIterator operator++(int) noexcept {
      assert(node_);
      auto copy_(*this);
      ++(*this);
      return copy_;
    }

    [[nodiscard]] reference operator*() const noexcept {
      assert(node_);
      return node_->value_;
    }
    [[nodiscard]] pointer operator->() const noexcept {
      assert(node_);
      return &node_->value_;
    }
  };

 private:
  Node head_;
  size_t size_ = 0;

  template <typename I>
  void reassign(I begin_, I end_) {
    SingleLinkedList<Type> temp_;
    Node* curr_ = &temp_.head_;
    while (begin_ != end_) {
      Node* next_node = new Node(*begin_, nullptr);
      curr_->next_node_ = next_node;
      curr_ = curr_->next_node_;
      ++begin_;
      ++temp_.size_;
    }
    swap(temp_);
  }

 public:
  using Iterator = BasicIterator<Type>;
  using ConstIterator = BasicIterator<const Type>;

  SingleLinkedList(std::initializer_list<Type> values) {
    reassign(values.begin(), values.end());
  }

  SingleLinkedList(const SingleLinkedList& other) {
    reassign(other.begin(), other.end());
  }

  SingleLinkedList& operator=(const SingleLinkedList& rhs) {
    reassign(rhs.begin(), rhs.end());
    return *this;
  }

  // Обменивает содержимое списков за время O(1)
  void swap(SingleLinkedList& other) noexcept {
    if (*this != other && !other.IsEmpty()) {
      std::swap(head_.next_node_, other.head_.next_node_);
      std::swap(size_ , other.size_);
    }
  }

  [[nodiscard]] Iterator begin() noexcept {
    return IsEmpty() && head_ ? Iterator(nullptr) : Iterator(head_.next_node_);
  }

  [[nodiscard]] Iterator end() noexcept { return Iterator(nullptr); }

  [[nodiscard]] ConstIterator begin() const noexcept {
    return IsEmpty() && head_ ? ConstIterator(nullptr)
                              : ConstIterator(head_.next_node_);
  }

  [[nodiscard]] ConstIterator end() const noexcept {
    return ConstIterator(nullptr);
  }

  [[nodiscard]] ConstIterator cbegin() const noexcept {
    return IsEmpty() && head_ ? ConstIterator(nullptr)
                              : ConstIterator(head_.next_node_);
  }

  [[nodiscard]] ConstIterator cend() const noexcept {
    return ConstIterator(nullptr);
  }

  SingleLinkedList() = default;

  [[nodiscard]] size_t GetSize() const noexcept { return size_; }

  [[nodiscard]] bool IsEmpty() const noexcept {
    return static_cast<int>(GetSize()) == 0;
  }

  void PushFront(const Type &value) {
    head_.next_node_ = new Node(value, head_.next_node_);
    ++size_;
  }

  void Clear() {
    while (head_) {
      Node *next = head_.next_node_->next_node_;
      delete head_.next_node_;
      head_.next_node_ = next;
    }
    size_ = 0;
  }

  ~SingleLinkedList() { Clear(); }
};

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
  lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs,
                const SingleLinkedList<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs,
                const SingleLinkedList<Type>& rhs) {
  return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs,
               const SingleLinkedList<Type>& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                      rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs,
                const SingleLinkedList<Type>& rhs) {
  return ((lhs == rhs || lhs < rhs));
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs,
               const SingleLinkedList<Type>& rhs) {
  return !(lhs <= rhs);
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs,
                const SingleLinkedList<Type>& rhs) {
  return !(lhs < rhs) ;
}

