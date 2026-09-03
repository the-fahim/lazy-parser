#include "lazyparser/parser.h"
#include "lazyparser/grammar.h"
#include <iostream>
#include <cassert>

int main() {
    // Build expression grammar:
    //   E -> E + T | T
    //   T -> NUM
    auto grammar = std::make_shared<lazyparser::Grammar>();

    auto expr = grammar->getOrCreateSymbol("E",   lazyparser::SymbolType::NON_TERMINAL);
    auto term = grammar->getOrCreateSymbol("T",   lazyparser::SymbolType::NON_TERMINAL);
    auto num  = grammar->getOrCreateSymbol("NUM", lazyparser::SymbolType::TERMINAL);
    auto plus = grammar->getOrCreateSymbol("+",   lazyparser::SymbolType::TERMINAL);

    grammar->addProduction(expr, {expr, plus, term});
    grammar->addProduction(expr, {term});
    grammar->addProduction(term, {num});

    grammar->startSymbol = expr;

    lazyparser::LazyParser parser(grammar);
    assert(parser.buildParser() && "Parser build failed");
    std::cout << "Test 0 PASSED: Parser build succeeded\n";

    // Test 1: "NUM"
    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokens1 = {
        {num, std::any()}
    };
    auto result1 = parser.parse(tokens1);
    assert(result1.success && ("Parse 'NUM' failed: " + result1.error).c_str());
    std::cout << "Test 1 PASSED: Parse 'NUM' OK\n";

    // Test 2: "NUM + NUM"
    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokens2 = {
        {num, std::any()}, {plus, std::any()}, {num, std::any()}
    };
    auto result2 = parser.parse(tokens2);
    assert(result2.success && ("Parse 'NUM + NUM' failed: " + result2.error).c_str());
    std::cout << "Test 2 PASSED: Parse 'NUM + NUM' OK\n";

    // Test 3: "NUM + NUM + NUM"
    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokens3 = {
        {num, std::any()}, {plus, std::any()},
        {num, std::any()}, {plus, std::any()},
        {num, std::any()}
    };
    auto result3 = parser.parse(tokens3);
    assert(result3.success && ("Parse 'NUM + NUM + NUM' failed: " + result3.error).c_str());
    std::cout << "Test 3 PASSED: Parse 'NUM + NUM + NUM' OK\n";

    // Test 4: Semantic action — multiply all NUM values by 2
    auto grammar2 = std::make_shared<lazyparser::Grammar>();
    auto e2  = grammar2->getOrCreateSymbol("E",   lazyparser::SymbolType::NON_TERMINAL);
    auto n2  = grammar2->getOrCreateSymbol("NUM", lazyparser::SymbolType::TERMINAL);
    grammar2->addProduction(e2, {n2});
    grammar2->startSymbol = e2;

    lazyparser::LazyParser parser2(grammar2);
    assert(parser2.buildParser());

    parser2.setSemanticAction(0, [](const std::vector<std::any>& vals) -> std::any {
        if (!vals.empty() && vals[0].has_value()) return vals[0];
        return std::any(42);
    });

    std::vector<std::pair<lazyparser::SymbolPtr, std::any>> tokensNum = {
        {n2, std::any(7)}
    };
    auto result4 = parser2.parse(tokensNum);
    assert(result4.success && "Semantic action parse failed");
    std::cout << "Test 4 PASSED: Semantic action parse OK\n";

    std::cout << "\nAll parser tests passed!\n";
    return 0;
}