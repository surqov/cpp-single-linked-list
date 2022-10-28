#pragma once
#include <cstddef>
#include <string>
#include <utility>
#include <iterator>
#include <algorithm>

template <typename Type>
class SingleLinkedList {
	struct Node {
        Node() = default;
        Node(const Type& val, Node* next)
            : value(val), 
              next_node(next) {
        }
        Type value;
        Node* next_node = nullptr;
    };
    template <typename ValueType>
    class BasicIterator {
	  private:
	    friend class SingleLinkedList;
        Node* node_ = nullptr;
        explicit BasicIterator(Node* node) : node_(node) { }
  	  public:
        using iterator_category = std::forward_iterator_tag; // Категория итератора — forward iterator
        using value_type = Type; // Тип элементов, по которым перемещается итератор
        using difference_type = std::ptrdiff_t; // Тип, используемый для хранения смещения между итераторами
        using pointer = ValueType*; // Тип указателя на итерируемое значение
        using reference = ValueType&; // Тип ссылки на итерируемое значение

        BasicIterator() = default;

        BasicIterator(const BasicIterator<Type>& other) noexcept : node_(other.node_) { }

        BasicIterator& operator=(const BasicIterator& rhs) = default;

		[[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ != rhs.node_;
        }

        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return node_ != rhs.node_;
        }

        BasicIterator& operator++() noexcept {
            if (node_) {
                node_ = node_->next_node;
            }
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            BasicIterator temp(*this);
            ++(*this);
            return temp;
        }

        [[nodiscard]] reference operator*() const noexcept {
             return node_->value;
        }

        [[nodiscard]] pointer operator->() const noexcept {
            return &node_->value;
        }
  };
  private:
    Node head_;
    size_t size_=0;
    Node* head_ptr_  = &head_;
    template <typename I>
    void reassign(I begin_, I end_) {
		SingleLinkedList<Type> curr;
        Node** ptr = &curr.head_.next_node;
        while (begin_ != end_) {
            *ptr = new Node(*begin_, nullptr);
            ++curr.size_;
            ptr = &((*ptr)->next_node);
            ++begin_;
        }
        swap(curr);
	}
  public:
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using Iterator = BasicIterator<Type>;
	using ConstIterator = BasicIterator<const Type>;

	SingleLinkedList() : head_(Node()), size_(0) { }
	
	SingleLinkedList(std::initializer_list<Type> values) {
		reassign(values.begin(), values.end());
	}
	
	SingleLinkedList(const SingleLinkedList& other) {
		reassign(other.begin(), other.end());
	}
	
	SingleLinkedList& operator=(const SingleLinkedList& rhs) {
		if (*this != rhs) {
			SingleLinkedList rhs_copy(rhs);
			swap(rhs_copy);
		}
		return *this;
	}
		
	void swap(SingleLinkedList& other) noexcept {
		std::swap(head_.next_node, other.head_.next_node);
		std::swap(size_ , other.size_);
	}

	[[nodiscard]] Iterator begin() noexcept {
		return IsEmpty() ? Iterator(nullptr) : Iterator(head_.next_node);
	}
	
	[[nodiscard]] Iterator end() noexcept {
		return Iterator(nullptr);
	}
	
	[[nodiscard]] ConstIterator begin() const noexcept {
		return IsEmpty() ? Iterator(nullptr) : ConstIterator(head_.next_node);
	}
	
	[[nodiscard]] ConstIterator end() const noexcept {
		return ConstIterator(nullptr);
	}
	
	[[nodiscard]] ConstIterator cbegin() const noexcept {
		return IsEmpty() ? Iterator(nullptr) : ConstIterator(head_.next_node);
	}
	
	[[nodiscard]] ConstIterator cend() const noexcept {
		return ConstIterator(nullptr);
	}
		
	[[nodiscard]] Iterator before_begin() noexcept {        
		return Iterator(head_ptr_);
	}
	
	[[nodiscard]] ConstIterator cbefore_begin() const noexcept {        
		return ConstIterator(head_ptr_);;
	}
	
	[[nodiscard]] ConstIterator before_begin() const noexcept {        
		return ConstIterator(head_ptr_);
	}
			
	[[nodiscard]] size_t GetSize() const noexcept {
		return size_;
	}
	
	[[nodiscard]] bool IsEmpty() const noexcept {
			return size_ == 0;
		}
		
	void PushFront(const Type& value) {
		head_.next_node = new Node(value, head_.next_node);
		++size_;
	} 
		
	void Clear() noexcept {
		if (IsEmpty()) {
			return;
		}
		while (head_.next_node != nullptr) {
			Node* temp = head_.next_node->next_node;
			delete head_.next_node;
			head_.next_node = temp;
		}
		size_ = 0;
	}
		
	Iterator InsertAfter(ConstIterator pos, const Type& value) {
		auto &prv_node = pos.node_;
		prv_node->next_node = new Node(value, prv_node->next_node);
		++size_;
		return Iterator(prv_node->next_node);  
	}
	
  	void PopFront() noexcept {
        if (IsEmpty()) {
            return;
        }
        Node* temp = head_.next_node->next_node;
        delete head_.next_node;
        head_.next_node = temp;
        --size_;
    }
	
	Iterator EraseAfter(ConstIterator pos) noexcept {
		if (IsEmpty()) {
			return Iterator(nullptr);
		}
		Node *const node_to_erase = pos.node_->next_node;
		--size_;
		
		Node *const node_after_erased= node_to_erase->next_node;
		delete node_to_erase;
		pos.node_->next_node = node_after_erased;
		return Iterator(node_after_erased);
	}
		
	~SingleLinkedList() {
		Clear();
	}
  };

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) && lhs != rhs);
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs < rhs || lhs == rhs);
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs > rhs || lhs == rhs);
}
