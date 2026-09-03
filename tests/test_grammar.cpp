#include "lazyparser/grammar.h"
#include <iostream>
#include <cassert>

int main() {
    // Test 1: Symbol creation
    auto grammar = std::make_shared<lazyparser::Grammar>();

    auto s1 = grammar->getOrCreateSymbol("E",   lazyparser::SymbolType::NON_TERMINAL);
    auto s2 = grammar->getOrCreateSymbol("NUM", lazyparser::SymbolType::TERMINAL);
    auto s3 = grammar->getOrCreateSymbol("+",   lazyparser::SymbolType::TERMINAL);

    assert(s1 != nullptr && "Symbol E creation failed");
    assert(s2 != nullptr && "Symbol NUM creation failed");
    assert(s3 != nullptr && "Symbol + creation failed");

    std::cout << "Test 1 PASSED: Symbol creation works\n";

    // Test 2: getOrCreateSymbol returns the same ptr for the same name
    auto s1b = grammar->getOrCreateSymbol("E", lazyparser::SymbolType::NON_TERMINAL);
    assert(s1 == s1b && "Same symbol should return the same pointer");
    std::cout << "Test 2 PASSED: Symbol identity is preserved\n";

    // Test 3: Production addition
    grammar->addProduction(s1, {s1, s3, s1});
    assert(grammar->getProductionCount() == 1 && "Production count incorrect");
    std::cout << "Test 3 PASSED: Production addition works (count: "
              << grammar->getProductionCount() << ")\n";

    // Test 4: Symbol counts
    assert(grammar->getSymbolCount()      >= 3 && "Symbol count incorrect");
    assert(grammar->getTerminalCount()    >= 2 && "Terminal count incorrect");
    assert(grammar->getNonTerminalCount() >= 1 && "Non-terminal count incorrect");

    std::cout << "Test 4 PASSED: Symbol counts correct\n";
    std::cout << "  Total symbols:  " << grammar->getSymbolCount()      << "\n";
    std::cout << "  Terminals:      " << grammar->getTerminalCount()     << "\n";
    std::cout << "  Non-terminals:  " << grammar->getNonTerminalCount()  << "\n";

    // Test 5: Production toString
    auto prod = grammar->productions[0];
    std::string repr = prod->toString();
    assert(!repr.empty() && "Production toString() should not be empty");
    std::cout << "Test 5 PASSED: Production toString = \"" << repr << "\"\n";

    std::cout << "\nAll grammar tests passed!\n";
    return 0;
}