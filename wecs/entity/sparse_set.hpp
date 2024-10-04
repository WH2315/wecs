#pragma once

#include "wecs/entity/entity.hpp"
#include "wecs/config/config.hpp"
#include <array>
#include <limits>
#include <memory>
#include <vector>
#include <utility>

namespace wecs {

namespace internal {

template <typename SparseSet>
struct SparseSetIterator {
    using container_type = typename SparseSet::packed_container_type;
    using pointer = typename container_type::const_pointer;
    using reference = typename container_type::const_reference;
    using difference_type = typename container_type::difference_type;
    using value_type = typename container_type::value_type;
    using iterator_category = std::random_access_iterator_tag;

    constexpr SparseSetIterator() : packed_{}, offset_{} {}

    constexpr SparseSetIterator(const container_type& ref, const difference_type idx)
        : packed_{&ref}, offset_{idx} {}

    constexpr pointer data() const {
        return packed_ ? packed_->data() : nullptr;
    }

    constexpr difference_type index() const {
        return offset_ - 1;
    }

    constexpr reference operator[](const difference_type value) const {
        return packed_->data()[index() - value];
    }

    constexpr pointer operator->() const {
        return std::addressof(operator[](0));
    }

    constexpr reference operator*() const {
        return *operator->();
    }

    constexpr SparseSetIterator& operator++() {
        return --offset_, *this;
    }

    constexpr SparseSetIterator& operator--() {
        return ++offset_, *this;
    }

    constexpr SparseSetIterator operator++(int) {
        SparseSetIterator copy = *this;
        return ++(*this), copy;
    }

    constexpr SparseSetIterator operator--(int) {
        SparseSetIterator copy = *this;
        return operator--(), copy;
    }

    constexpr SparseSetIterator& operator+=(const difference_type value) {
        return offset_ -= value, *this;
    }

    constexpr SparseSetIterator& operator-=(const difference_type value) {
        return *this += -value;
    }

    constexpr SparseSetIterator operator+(const difference_type value) const {
        SparseSetIterator copy = *this;
        return copy += value;
    }

    constexpr SparseSetIterator operator-(const difference_type value) const {
        return *this + -value;
    }

private:
    const container_type* packed_;
    difference_type offset_;
};

template <typename SparseSet>
constexpr std::ptrdiff_t operator-(const SparseSetIterator<SparseSet>& lhs,
                                   const SparseSetIterator<SparseSet>& rhs) {
    return rhs.index() - lhs.index();
}

template <typename SparseSet>
constexpr bool operator==(const SparseSetIterator<SparseSet>& lhs,
                          const SparseSetIterator<SparseSet>& rhs) {
    return lhs.index() == rhs.index();
}

template <typename SparseSet>
constexpr bool operator!=(const SparseSetIterator<SparseSet>& lhs,
                          const SparseSetIterator<SparseSet>& rhs) {
    return !(lhs == rhs);
}

} // namespace internal

template <typename EntityType, size_t PageSize>
class BasicSparseSet {
public:
    using traits_type = EntityTraits<EntityType>;
    using entity_type = typename traits_type::entity_type;
    using page_type = std::array<size_t, PageSize>;
    using packed_container_type = std::vector<entity_type>;
    using sparse_container_type = std::vector<page_type>;
    static constexpr typename page_type::value_type npos = std::numeric_limits<typename page_type::value_type>::max();
    using iterator = internal::SparseSetIterator<BasicSparseSet>;
    using const_iterator = iterator;

    virtual ~BasicSparseSet() = default;

    auto insert(EntityType value) {
        auto entity = to_entity(value);
        WECE_ASSERT(traits_type::entity_mask != entity, "invalid entity");
        packed_.push_back(to_integral(entity));
        assure(page(entity))[offset(entity)] = packed_.size() - 1u;
        return traits_type::construct(
            static_cast<typename traits_type::entity_type>(packed_.size() - 1u),
            0
        );
    }

    virtual void remove(EntityType value) {
        WECE_ASSERT(contain(value), "entity not found");
        auto entity = to_entity(value);
        auto& ref = sparse_ref(entity);
        auto pos = ref;
        packed_[pos] = std::move(packed_.back());
        sparse_ref(to_entity(packed_[pos])) = pos;
        ref = npos;
        packed_.pop_back();
    }

    bool contain(EntityType value) const {
        auto entity = to_entity(value);
        auto page = this->page(entity);
        if (page >= sparse_.size()) {
            return false;
        }
        auto pos = sparse_[page][offset(entity)];
        return pos != npos && packed_[pos] == to_integral(entity);
    }

    size_t index(EntityType value) {
        auto entity = to_entity(value);
        auto page = this->page(entity);
        return page < sparse_.size() ? sparse_[page][offset(entity)] : npos;
    }

    auto& swap(EntityType lhs, EntityType rhs) {
        auto& ref1 = sparse_ref(to_entity(lhs));
        auto& ref2 = sparse_ref(to_entity(rhs));
        std::swap(packed_[ref1], packed_[ref2]);
        std::swap(ref1, ref2);
        return packed_[ref1];
    }

public:
    bool empty() const noexcept {
        return packed_.empty();
    }

    auto size() const noexcept {
        return packed_.size();
    }

    auto capacity() const noexcept {
        return packed_.capacity();
    }

    void reserve(size_t capacity) {
        packed_.reserve(capacity);
    }

    void clear() noexcept {
        packed_.clear();
        sparse_.clear();
    }

    iterator begin() const noexcept {
        return {packed_, static_cast<typename iterator::difference_type>(packed_.size())};
    }

    iterator end() const noexcept {
        return {packed_, 0};
    }

    const_iterator cbegin() noexcept {
        return begin();
    }

    const_iterator cend() noexcept {
        return end();
    }

    const auto& packed() const noexcept { return packed_; }
    auto& packed() noexcept { return std::as_const(*this).packed(); } 

    const_iterator find(EntityType value) noexcept {
        return {packed_, static_cast<typename iterator::difference_type>(index(value))};
    }

private:
    size_t page(entity_type entity) const {
        return entity / PageSize;
    }

    size_t offset(entity_type entity) const {
        return entity % PageSize;
    }

    auto& assure(size_t page) {
        if (page >= sparse_.size()) {
            const size_t old = sparse_.size();
            sparse_.resize(page + 1);
            for (size_t i = old; i < sparse_.size(); i++) {
                std::uninitialized_fill(std::begin(sparse_[i]), std::end(sparse_[i]), npos);
            }
        }
        return sparse_[page];
    }

    auto* sparse_ptr(const entity_type entity) {
        auto page = this->page(entity);
        return page < sparse_.size() ? &sparse_[page][offset(entity)] : nullptr;
    }

    auto& sparse_ref(const entity_type entity) {
        WECE_ASSERT(sparse_ptr(entity), "invalid entity");
        return sparse_[page(entity)][offset(entity)];
    }

private:
    packed_container_type packed_;
    sparse_container_type sparse_;
};

} // namespace wecs