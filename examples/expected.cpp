#include "../src/expected.h"
#include <iostream>
#include <print>
#include <string>

using monads::expected;
using std::cin;
using std::isdigit;
using std::print;
using std::println;
using std::string;
using std::unexpected;

expected<int, string> pass_fail(string in) {
    for (auto c : in) {
        if (!isdigit(c)) {
            return unexpected(string("Input is not a number"));
        }
    }
    return std::stoi(in);
}
int main() {
    string input;
    print("Enter a number: ");
    cin >> input;
    auto [val, error] = pass_fail(input);
    if (auto [err_val, ok] = error; ok) {
        println("Error");
        println("{}", *err_val);
    } else if (auto [val_val, ok] = val; ok) {
        println("Value");
        println("{}", *val_val);
    }
}
