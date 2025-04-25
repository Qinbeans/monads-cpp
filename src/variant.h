#pragma once
#include "optional.h"
#include <type_traits>
#include <variant>

namespace monads {
template <typename... T> class variant : public std::variant<T...> {
  public:
    using std::variant<T...>::variant;

    template <std::size_t I> auto get() {
        using variant_type = std::variant_alternative_t<I, std::variant<T...>>;
        using result_type = std::pair<variant_type *, bool>;

        if (this->index() == I) {
            return monads::optional<variant_type>(std::get<I>(*this));
        } else {
            return monads::optional<variant_type>();
        }
    }
};
} // namespace monads

namespace std {
template <typename... T>
struct tuple_size<monads::variant<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)> {};

template <std::size_t I, typename... T>
struct tuple_element<I, monads::variant<T...>> {
    using variant_type = std::variant_alternative_t<I, std::variant<T...>>;
    using type = monads::optional<variant_type>;
};

template <std::size_t I, typename... T> auto get(monads::variant<T...> &v) {
    return v.template get<I>();
}
} // namespace std