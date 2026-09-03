#include "lazyparser/parser.h"
#include "lazyparser/grammar.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    // Build a simple left-recursive expression grammar:
    //   E -> E + T
    //   E -> T
    //   T -> NUM
    auto grammar = std::make_shared<lazyparser::Grammar>();

    auto expr = grammar->getOrCreateSymbol("E",   lazyparser::SymbolType::NON_TERMINAL);
    auto term = grammar->getOrCreateSymbol("T",   lazyparser::SymbolType::NON_TERMINAL);
    auto num  = grammar->getOrCreateSymbol("NUM", lazyparser::SymbolType::TERMINAL);
    auto plus = grammar->getOrCreateSymbol("+",   lazyparser::SymbolType::TERMINAL);

    grammar->addProduction(expr, {expr, plus, term});
    grammar->addProduction(expr, {term});
    grammar->addProduction(term, {num});

    // S' -> E is auto-created by buildParser()
    grammar->startSymbol = expr;

    lazyparser::LazyParser parser(grammar);

    if (!parser.buildParser()) {
        std::cerr << "Failed to build parser!\n";
        return 1;
    }

    std::cout << "Parser built successfully!\n";
    parser.printStatistics();

    // Test 1: "NUM"
    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokens1 = {
        {num, std::any()}
    };
    auto result1 = parser.parse(tokens1);
    std::cout << "Parse 'NUM':         "
              << (result1.success ? "OK" : ("FAIL: " + result1.error))
              << "\n";

    // Test 2: "NUM + NUM"
    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokens2 = {
        {num, std::any()}, {plus, std::any()}, {num, std::any()}
    };
    auto result2 = parser.parse(tokens2);
    std::cout << "Parse 'NUM + NUM':   "
              << (result2.success ? "OK" : ("FAIL: " + result2.error))
              << "\n";

    // Test 3: "NUM + NUM + NUM"
    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokens3 = {
        {num, std::any()}, {plus, std::any()},
        {num, std::any()}, {plus, std::any()},
        {num, std::any()}
    };
    auto result3 = parser.parse(tokens3);
    std::cout << "Parse 'NUM+NUM+NUM': "
              << (result3.success ? "OK" : ("FAIL: " + result3.error))
              << "\n";

    return (result1.success && result2.success && result3.success) ? 0 : 1;
}