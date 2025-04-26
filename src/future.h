#pragma once
#include "optional.h"
#include <coroutine>
#include <future>
#include <exception>

namespace monads {
template <typename T> class future {
  public: // Make promise_type public so std::coroutine_traits can access it
    // Define promise_type for coroutine support
    struct promise_type {
        T result;
        std::exception_ptr exception = nullptr;
        
        future<T> get_return_object() {
            return future<T>(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; } // Changed to always suspend at the end
        
        void unhandled_exception() {
            exception = std::current_exception();
        }
        
        void return_value(T value) {
            result = std::move(value);
        }
        
        // Support for co_yield
        std::suspend_always yield_value(T value) {
            result = std::move(value);
            return {};
        }
    };
    
  private:
    mutable bool _consumed = false;
    mutable monads::optional<T> _cached_value;
    mutable monads::optional<std::string> _cached_error;
    mutable bool _ready = false;

    std::coroutine_handle<promise_type> _coroutine = nullptr;
    std::shared_ptr<std::future<T>> _future;

  public:
    // Constructor for coroutines
    future(std::coroutine_handle<promise_type> handle) : _coroutine(handle) {}
    
    // Constructor that takes a std::future<T>
    future(std::future<T> &&other) : _future(std::make_shared<std::future<T>>(std::move(other))) {}
    
    // Default constructor
    future() = default;
    
    // Copy constructor
    future(const future& other) 
        : _consumed(other._consumed),
          _cached_value(other._cached_value),
          _cached_error(other._cached_error),
          _ready(other._ready),
          _coroutine(other._coroutine),
          _future(other._future) {}
    
    // Copy assignment
    future& operator=(const future& other) {
        if (this != &other) {
            _consumed = other._consumed;
            _cached_value = other._cached_value;
            _cached_error = other._cached_error;
            _ready = other._ready;
            _coroutine = other._coroutine;
            _future = other._future;
        }
        return *this;
    }
    
    // Move constructor
    future(future&& other) noexcept 
        : _consumed(other._consumed),
          _cached_value(std::move(other._cached_value)),
          _cached_error(std::move(other._cached_error)),
          _ready(other._ready),
          _coroutine(other._coroutine),
          _future(std::move(other._future)) {
        other._coroutine = nullptr;
    }
    
    // Move assignment
    future& operator=(future&& other) noexcept {
        if (this != &other) {
            _consumed = other._consumed;
            _cached_value = std::move(other._cached_value);
            _cached_error = std::move(other._cached_error);
            _ready = other._ready;
            _coroutine = other._coroutine;
            _future = std::move(other._future);
            other._coroutine = nullptr;
        }
        return *this;
    }
    
    // Destructor
    ~future() {
        if (_coroutine && _coroutine.done()) {
            _coroutine.destroy();
        }
    }

    bool is_ready() const {
        bool status = false;
        
        if (_coroutine) {
            status = _coroutine.done();
        } else if (_future && _future->valid()) {
            auto wait_status = _future->wait_for(std::chrono::seconds(0));
            status = (wait_status == std::future_status::ready);
        }
        
        // Update the ready flag - this makes the pointer always reflect current state
        _ready = status;
        return status;
    }

    // Get methods for structured binding with const reference
    template <std::size_t I> decltype(auto) get() const {
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
            
                if (_coroutine) {
                    // Get value from coroutine
                    if (_coroutine.promise().exception) {
                        std::rethrow_exception(_coroutine.promise().exception);
                    }
                    _cached_value = _coroutine.promise().result;
                } else if (_future) {
                    // Get value from std::future
                    _cached_value = _future->get();
                }
            
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
        // Ready pointer - return a pointer to the internal ready state
        else if constexpr (I == 2) {
            // Always check ready status before returning the pointer
            // This ensures the pointer always reflects the current state
            is_ready(); // This updates _ready internally
            return &_ready;
        } else {
            static_assert(I < 3, "Index out of bounds for monads::future");
            if constexpr (I == 0) {
                return monads::optional<T>();
            } else if constexpr (I == 1) {
                return monads::optional<std::string>();
            } else {
                static bool default_ready = false;
                return &default_ready;
            }
        }
    }

    // Get methods for structured binding (non-const version)
    template <std::size_t I> decltype(auto) get() {
        return const_cast<const future<T>*>(this)->template get<I>();
    }

    static future<T> make_ready(T value) {
        std::promise<T> promise;
        promise.set_value(value);
        // Move from the std::future into our monads::future
        return future<T>(std::move(promise.get_future()));
    }

    // Wait for the future to complete
    void wait() {
        if (_coroutine) {
            // No explicit wait for coroutines, but we'll just check the done status
            while (!_coroutine.done()) {
                std::this_thread::yield();
            }
        } else if (_future && _future->valid()) {
            _future->wait();
        }
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
        std::conditional_t<I == 1, monads::optional<std::string>, const bool*>>;
};

template <std::size_t I, typename T> decltype(auto) get(monads::future<T> &f) {
    return f.template get<I>();
}

template <std::size_t I, typename T> decltype(auto) get(const monads::future<T> &f) {
    return f.template get<I>();
}

// Specialization for coroutine_traits
template <typename T, typename... Args>
struct coroutine_traits<monads::future<T>, Args...> {
    using promise_type = typename monads::future<T>::promise_type;
};
} // namespace std