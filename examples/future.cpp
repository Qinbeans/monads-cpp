#include "../src/future.h"
#include <chrono>
#include <iostream>
#include <print>
#include <thread>

using monads::future;
using std::cin;
using std::print;
using std::println;
using std::string;

// This function creates a truly asynchronous coroutine
future<int> async_wait_ms(int ms_wait) {
    // Launch a background thread to do the work
    std::thread([ms_wait]() {
        println("Background thread starting...");
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_wait));
        println("Background thread completed after {} ms", ms_wait);
    }).detach();

    // Return immediately with the expected result
    co_return ms_wait;
}

// This is a regular coroutine that will execute in the calling thread
future<int> wait_ms(int ms_wait) {
    println("Starting sleep for {} ms", ms_wait);
    std::this_thread::sleep_for(std::chrono::milliseconds((ms_wait)));
    println("Finished sleep after {} ms", ms_wait);
    co_return ms_wait;
}

int main() {
    int ms_wait;
    print("Enter a time in milliseconds: ");
    cin >> ms_wait;

    println("\n[Example 1: Blocking with auto-destructuring]\n");
    println("Starting synchronous operation...");
    // Regular blocking wait with structured binding
    auto [val1, err1, ready1] = wait_ms(ms_wait / 2);

    println("Coroutine finished, ready: {}", *ready1);

    if (auto [val_ok, ok] = val1; ok) {
        println("Value: {}", *val_ok);
    }

    println("\n[Example 2: Async with polling]\n");
    println("Starting asynchronous operation...");
    // Start async operation
    auto future_result = async_wait_ms(ms_wait);

    // Destructure to get components, especially the ready pointer
    auto [val2, err2, ready2] = future_result;

    println("Operation started in background, main thread continues");

    // Demonstrate polling in a loop
    int dots = 0;
    while (!*ready2) {
        print(".");
        std::cout.flush();
        dots++;

        // Do some work in the main thread
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Check the ready pointer - it will automatically reflect the current
        // status without needing to re-destructure the future
        if (dots >= 10) {
            println("");
            println("Still waiting, ready status: {}", *ready2);
            dots = 0;
        }
    }

    println("\nOperation completed, ready status: {}", *ready2);

    // Access the value
    if (auto [val_ok, ok] = val2; ok) {
        println("Result: {}", *val_ok);
    }

    println("Done!");
    return 0;
}
