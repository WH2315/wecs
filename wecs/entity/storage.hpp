#pragma once

#include "wecs/entity/sparse_set.hpp"
#include "wecs/entity/component.hpp"
#include "wecs/config/config.hpp"

namespace wecs {

namespace internal {

template <typename Container>
struct StorageIterator {
    friend StorageIterator<const Container>;

    using container_type = std::remove_const_t<Container>;
    using alloc_traits = std::allocator_traits<typename container_type::allocator_type>;
    using iterator_traits = std::iterator_traits<std::conditional_t<std::is_const_v<Container>,
        typename alloc_traits::template rebind_traits<typename std::pointer_traits<typename container_type::value_type>::element_type>::const_pointer,
        typename alloc_traits::template rebind_traits<typename std::pointer_traits<typename container_type::value_type>::element_type>::pointer>>;
    using pointer = typename iterator_traits::pointer;
    using reference = typename iterator_traits::reference;
    using difference_type = typename iterator_traits::difference_type;
    using value_type = typename iterator_traits::value_type;
    using iterator_category = std::random_access_iterator_tag;

    constexpr StorageIterator() : payload_{}, offset_{} {}

    constexpr StorageIterator(Container* ref, const difference_type idx)
        : payload_{ref}, offset_{idx} {}
    
    constexpr difference_type index() const {
        return offset_ - 1;
    }

    constexpr reference operator[](const difference_type value) const {
        const auto pos = index() - value;
        constexpr auto page_size = ComponentTraits<value_type>::page_size;
        return (*payload_)[pos / page_size][pos % page_size];
    }

    constexpr pointer operator->() const {
        return std::addressof(operator[](0));
    }

    constexpr reference operator*() const {
        return operator[](0);
    }

    constexpr StorageIterator& operator++() {
        return --offset_, *this;
    }

    constexpr StorageIterator& operator--() {
        return ++offset_, *this;
    }

    constexpr StorageIterator operator++(int) {
        StorageIterator copy = *this;
        return ++(*this), copy;
    }

    constexpr StorageIterator operator--(int) {
        StorageIterator copy = *this;
        return operator--(), copy;
    }

    constexpr StorageIterator& operator+=(const difference_type value) {
        return offset_ -= value, *this;
    }

    constexpr StorageIterator& operator-=(const difference_type value) {
        return *this += -value;
    }

    constexpr StorageIterator operator+(const difference_type value) const {
        StorageIterator copy = *this;
        return copy += value;
    }

    constexpr StorageIterator operator-(const difference_type value) const {
        return *this + -value;
    }

private:
    Container* payload_;
    difference_type offset_;
};

template <typename Lhs, typename Rhs>
constexpr std::ptrdiff_t operator-(const StorageIterator<Lhs>& lhs,
                                   const StorageIterator<Rhs>& rhs) {
    return rhs.index() - lhs.index();
}

template <typename Lhs, typename Rhs>
constexpr bool operator==(const StorageIterator<Lhs>& lhs,
                          const StorageIterator<Rhs>& rhs) {
    return lhs.index() == rhs.index();
}

template <typename Lhs, typename Rhs>
constexpr bool operator!=(const StorageIterator<Lhs>& lhs,
                          const StorageIterator<Rhs>& rhs) {
    return !(lhs == rhs);
}

} // namespace internal

template <typename EntityType, typename Payload, size_t PageSize, typename Allocator>
class BasicStorage : public BasicSparseSet<EntityType, PageSize> {
public:
    using entity_type = typename EntityTraits<EntityType>::entity_type;
    using allocator_type = Allocator;
    using alloc_traits = std::allocator_traits<allocator_type>;
    using container_type = std::vector<typename alloc_traits::pointer, typename alloc_traits::template rebind_alloc<typename alloc_traits::pointer>>;
    using base_type = BasicSparseSet<EntityType, PageSize>;
    using component_traits = ComponentTraits<Payload>;
    using iterator = internal::StorageIterator<container_type>;
    using const_iterator = internal::StorageIterator<const container_type>;

    BasicStorage() {}
    ~BasicStorage() override {}

    template <typename... Args>
    auto emplace(EntityType value, Args&&... args) {
        WECS_ASSERT(!base_type::contain(value), "entity already exists");
        base_type::insert(value);
        auto index = base_type::index(value);
        return new (assure(index)) Payload{std::forward<Args>(args)...};
    }

    void remove(EntityType value) override {
        WECS_ASSERT(base_type::contain(value), "entity not found");
        auto index = base_type::index(value);
        payload_[index]->~Payload();
        std::swap(payload_[index], payload_[size() - 1]);
        base_type::remove(value);
    }

public:
    bool empty() const noexcept {
        return payload_.empty();
    }

    auto size() const noexcept {
        return payload_.size();
    }

    auto capacity() const noexcept {
        return payload_.capacity();
    }

    const_iterator cbegin() const noexcept {
        const auto pos = static_cast<typename iterator::difference_type>(size());
        return const_iterator{&payload_, pos};
    }

    const_iterator begin() const noexcept {
        return cbegin();
    }

    iterator begin() noexcept {
        const auto pos = static_cast<typename iterator::difference_type>(size());
        return iterator{&payload_, pos};
    }

    const_iterator cend() const noexcept {
        return const_iterator{&payload_, {}};
    }

    const_iterator end() const noexcept {
        return cend();
    }

    iterator end() noexcept {
        return {&payload_, {}};
    }

    const auto& payloads() const noexcept { return payload_; }
    auto& payloads() noexcept { return std::as_const(*this).payloads(); }

    const_iterator find(EntityType value) noexcept {
        if (base_type::contain(value)) {
            return {&payload_, static_cast<typename iterator::difference_type>(base_type::index(value)) + 1};
        } else {
            return end();
        }
    }

    void clear() noexcept {
        std::size_t i = 0;
        allocator_type allocator = get_allocator();
        while (i < size()) {
            payload_[i]->~Payload();
            allocator.deallocate(payload_[i], component_traits::page_size);
            payload_[i] = nullptr;
            i++;
        }
        payload_.clear();
        base_type::clear();
    }

private:
    auto assure(size_t index) {
        constexpr auto page_size = component_traits::page_size;
        const auto idx = index / page_size;
        if (idx >= payload_.size()) {
            const size_t old = payload_.size();
            allocator_type allocator = get_allocator();
            payload_.resize(idx + 1, nullptr);
            for (size_t i = old; i < payload_.size(); i++) {
                payload_[i] = alloc_traits::allocate(allocator, page_size);
            }
        }
        return payload_[idx] + index % page_size;
    }

    allocator_type get_allocator() const noexcept {
        return payload_.get_allocator();
    }

private:
    container_type payload_;
};

template <typename EntityType, size_t PageSize>
class BasicStorage<EntityType, EntityType, PageSize, void>
    : public BasicSparseSet<EntityType, PageSize> {
public:
    using traits_type = EntityTraits<EntityType>;
    using entity_type = typename traits_type::entity_type;
    using base_type = BasicSparseSet<EntityType, PageSize>;

    auto emplace() {
        length_++;
        if (length_ <= base_size()) {
            return EntityType{(base_type::packed()[length_ - 1])};
        } else {
            return base_type::insert(static_cast<EntityType>(base_size()));
        }
    }

    void remove(EntityType value) override {
        WECS_ASSERT(base_type::contain(value), "entity not found");
        auto& ref = base_type::swap(value, EntityType{base_type::packed()[length_ - 1]});
        ref = static_cast<entity_type>(traits_type::next(EntityType{ref}));
        length_--;
    }

public:
    bool empty() const noexcept {
        return length_ == 0;
    }

    auto size() const noexcept {
        return length_;
    }

    auto base_size() const noexcept {
        return base_type::size();
    }

    void clear() noexcept {
        base_type::clear();
        length_ = 0;
    }

private:
    size_t length_ = 0;
};

} // namespace wecs