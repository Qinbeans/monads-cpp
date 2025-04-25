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

future<int> wait_ms(int ms_wait) {
    std::this_thread::sleep_for(std::chrono::milliseconds((ms_wait)));
    println("ready");
    return future<int>::make_ready(ms_wait);
}

int main() {
    int ms_wait;
    print("Enter a time: ");
    cin >> ms_wait;

    auto [val, err, ready] = wait_ms(ms_wait);

    // Process results
    if (auto [val_val, ok] = val; ok) {
        println("val is {}", *val_val);
    } else {
        println("val is nullopt");
    }
}