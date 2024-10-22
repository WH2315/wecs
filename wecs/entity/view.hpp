#pragma once

#include "wecs/core/utility.hpp"
#include "wecs/core/type_list.hpp"

namespace wecs {

namespace internal {

template <typename View>
struct ViewIterator {
    using entity_type = typename View::entity_type;
    using pool_container_type = typename View::pool_container_type;
    using entity_container_type = typename View::entity_container_type;
    using size_type = typename entity_container_type::size_type;
    using difference_type = typename entity_container_type::difference_type;
    using iterator_category = std::random_access_iterator_tag;

    ViewIterator(pool_container_type pools, const entity_container_type& entities, size_type offset)
        : pools_{pools}, entities_{entities}, offset_{offset} {}

    ViewIterator& operator++() { return --offset_, *this; }

    ViewIterator& operator--() { return ++offset_, *this; }

    ViewIterator operator++(int) {
        ViewIterator copy = *this;
        return ++(*this), copy;
    }

    ViewIterator operator--(int) {
        ViewIterator copy = *this;
        return --(*this), copy;
    }

    ViewIterator& operator+=(const difference_type value) {
        return offset_ -= value, *this;
    }

    ViewIterator& operator-=(const difference_type value) {
        return offset_ += value, *this;
    }

    ViewIterator operator+(const difference_type value) const {
        ViewIterator copy = *this;
        return copy += value;
    }

    ViewIterator operator-(const difference_type value) const {
        ViewIterator copy = *this;
        return copy -= value;
    }

    bool operator==(const ViewIterator& other) const {
        return offset_ == other.offset_;
    }

    bool operator!=(const ViewIterator& other) const {
        return offset_ != other.offset_;
    }

    auto operator*() noexcept {
        auto entity = entities_[offset_ - 1];
        return std::apply([&entity](auto*... pool) {
            return std::tuple_cat(
                std::make_tuple(static_cast<entity_type>(entity)),
                std::forward_as_tuple((*pool)[static_cast<entity_type>(entity)])...
            );
        }, pools_);
    }

private:
    pool_container_type pools_;
    entity_container_type entities_;
    size_type offset_;
};

} // namespace internal

template <typename EntityType, typename Registry, typename... Types>
class View {
public:
    using view_list = TypeList<Types...>;
    template <typename Type>
    struct StorageFor {
        using type = constness_as_t<typename Registry::template storage_for_t<std::remove_const_t<Type>>, Type>*;
    };

    using entity_type = EntityType;
    using pool_container_type = type_list_to_tuple_t<type_list_transform_t<view_list, StorageFor>>;
    using entity_container_type = typename Registry::base_type::packed_container_type;
    using iterator = internal::ViewIterator<View>;
    using const_iterator = const iterator;

    View(pool_container_type pools, const entity_container_type& entities)
        : pools_{pools}, entities_{entities} {}
    
    View(pool_container_type pools, entity_container_type&& entities)
        : pools_{pools}, entities_{std::move(entities)} {}
    
public:
    size_t size() const noexcept {
        return entities_.size();
    }

    bool empty() const noexcept {
        return entities_.empty();
    }

    iterator begin() noexcept {
        return iterator{pools_, entities_, entities_.size()};
    }

    iterator end() noexcept {
        return iterator{pools_, entities_, 0};
    }

    auto& entities() const noexcept { return entities_; }

private:
    pool_container_type pools_;
    entity_container_type entities_;
};

} // namespace wecs