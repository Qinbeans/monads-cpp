#include "../src/optional.h"
#include <iostream>
#include <print>

using monads::optional;
using std::cin;
using std::isdigit;
using std::nullopt;
using std::print;
using std::println;
using std::string;

optional<int> pass_fail(string in) {
    for (auto c : in) {
        if (!isdigit(c)) {
            return nullopt;
        }
    }
    return stoi(in);
}

int main() {
    string input;
    print("Enter a number: ");
    cin >> input;
    auto [i, b] = pass_fail(input);
    if (b) {
        println("i is {}", *i);
    } else {
        println("i is nullopt");
    }
}