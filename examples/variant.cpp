#include "../src/variant.h"
#include <cctype>
#include <iostream>
#include <print>

using monads::variant;
using std::cin;
using std::print;
using std::println;
using std::stod;
using std::stoi;
using std::string;

variant<int, double> pass_fail(string in) {
    if (in.find(".") != string::npos) {
        return stod(in);
    } else {
        return stoi(in);
    }
}

int main() {
    string input;
    print("Enter a number: ");
    cin >> input;
    auto [i, d] = pass_fail(input);
    if (auto [i_val, ok] = i; ok) {
        println("i is {}", *i_val);
    }
    if (auto [d_val, ok] = d; ok) {
        println("d is {:.2f}", *d_val);
    }
}
