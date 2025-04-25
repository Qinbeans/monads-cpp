#pragma once
#include "optional.h"
#include <cstddef>
#include <expected>
#include <tuple>
#include <utility>

namespace monads {
template <typename T, typename E> class expected : public std::expected<T, E> {
  public:
    using std::expected<T, E>::expected;

    template <std::size_t I> decltype(auto) get() {
        if constexpr (I == 0) {
            return this->has_value() ? monads::optional<T>(this->value())
                                     : monads::optional<T>();
        } else if constexpr (I == 1) {
            return this->has_value() ? monads::optional<E>()
                                     : monads::optional<E>(this->error());
        } else {
            static_assert(I < 2, "Index out of bounds for monads::expected");
        }
    }
};
} // namespace monads

namespace std {
template <typename T, typename E>
struct tuple_size<monads::expected<T, E>>
    : std::integral_constant<std::size_t, 2> {};

template <std::size_t I, typename T, typename E>
struct tuple_element<I, monads::expected<T, E>> {
    using type =
        std::conditional_t<I == 0, monads::optional<T>, monads::optional<E>>;
};

template <std::size_t I, typename T, typename E>
decltype(auto) get(monads::expected<T, E> &e) {
    return e.template get<I>();
}
} // namespace std
