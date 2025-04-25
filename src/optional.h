#pragma once
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

namespace monads {
template <typename T> class optional : public std::optional<T> {
  public:
    using std::optional<T>::optional;

    // Value 0: pointer to value
    // Value 1: bool has_value
    template <std::size_t I> decltype(auto) get() {
        if constexpr (I == 0) {
            return this->has_value() ? &this->value()
                                     : static_cast<T *>(nullptr);
        } else if constexpr (I == 1) {
            return this->has_value();
        } else {
            static_assert(I < 2, "Index out of bounds for monads::expected");
        }
    }
};
} // namespace monads

namespace std {
template <typename T>
struct tuple_size<monads::optional<T>>
    : std::integral_constant<std::size_t, 2> {};

template <std::size_t I, typename T>
struct tuple_element<I, monads::optional<T>> {
    using type = std::conditional_t<I == 0, T *, bool>;
};

template <std::size_t I, typename T>
decltype(auto) get(monads::optional<T> &e) {
    return e.template get<I>();
}
} // namespace std
