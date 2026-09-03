#include "lazyparser/utils.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

int main() {
    // Test trim()
    assert(lazyparser::trim("  hello  ") == "hello");
    assert(lazyparser::trim("\t world\n") == "world");
    assert(lazyparser::trim("")           == "");
    std::cout << "Test 1 PASSED: trim() works correctly\n";

    // Test splitTokens()
    auto toks = lazyparser::splitTokens("E ::= E + T");
    assert(toks.size() == 5);
    assert(toks[0] == "E");
    assert(toks[1] == "::=");
    assert(toks[4] == "T");
    std::cout << "Test 2 PASSED: splitTokens() works correctly\n";

    std::cout << "\nLazyParser v1.0 — utility tests passed!\n";
    return 0;
}