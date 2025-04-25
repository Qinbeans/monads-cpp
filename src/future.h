#pragma once
#include "optional.h"
#include <future>

namespace monads {
template <typename T> class future : public std::future<T> {
  private:
    mutable bool _consumed = false;
    mutable monads::optional<T> _cached_value;
    mutable monads::optional<std::string> _cached_error;

  public:
    // Inherit constructors from std::future<T>
    using std::future<T>::future;

    // Constructor that takes a std::future<T> and moves from it
    future(std::future<T> &&other) : std::future<T>(std::move(other)) {}

    bool is_ready() const {
        if (!this->valid())
            return false;

        auto status = this->wait_for(std::chrono::seconds(0));
        return status == std::future_status::ready;
    }

    // Get methods for structured binding
    template <std::size_t I> decltype(auto) get() {
        // Value
        if constexpr (I == 0) {
            if (_consumed) {
                return _cached_value;
            }

            if (!is_ready()) {
                return monads::optional<T>();
            }

            try {
                _consumed = true;
                _cached_value = std::future<T>::get();
                return _cached_value;
            } catch (const std::exception &e) {
                _cached_error = e.what();
                return monads::optional<T>();
            }
        }
        // Error
        else if constexpr (I == 1) {
            return _cached_error;
        }
        // Ready
        else if constexpr (I == 2) {
            return is_ready();
        } else {
            static_assert(I < 3, "Index out of bounds for monads::future");
            if constexpr (I == 0) {
                return monads::optional<T>();
            } else if constexpr (I == 1) {
                return monads::optional<std::string>();
            } else {
                return false;
            }
        }
    }

    static future<T> make_ready(T value) {
        std::promise<T> promise;
        promise.set_value(value);
        // Move from the std::future into our monads::future
        return future<T>(std::move(promise.get_future()));
    }
};
} // namespace monads

namespace std {
template <typename T>
struct tuple_size<monads::future<T>> : std::integral_constant<std::size_t, 3> {
};

template <std::size_t I, typename T>
struct tuple_element<I, monads::future<T>> {
    using type = std::conditional_t<
        I == 0, monads::optional<T>,
        std::conditional_t<I == 1, monads::optional<std::string>, bool>>;
};

template <std::size_t I, typename T> decltype(auto) get(monads::future<T> &f) {
    return f.template get<I>();
}
} // namespace std