#include "lazyparser/parser.h"
#include "lazyparser/grammar.h"
#include "lazyparser/symbol.h"
#include "lazyparser/utils.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <any>
#include <memory>
#include <set>
#include <algorithm>

using namespace lazyparser;

// ---------------------------------------------------------------------------
// Grammar file parser
// Supported format:
//   # comment
//   LHS ::= RHS_sym1 RHS_sym2 | RHS_alt1
//   LHS ->  RHS_sym1 RHS_sym2
// ---------------------------------------------------------------------------

GrammarPtr parseGrammarFromFile(const std::string& filename) {
    auto grammar = std::make_shared<Grammar>();

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open grammar file: " << filename << "\n";
        return nullptr;
    }

    // First pass: collect all LHS names to identify non-terminals
    std::set<std::string> lhsNames;
    std::vector<std::string> rawLines;

    auto extractSep = [](const std::string& line) -> size_t {
        size_t pos = line.find("::=");
        if (pos != std::string::npos) return pos;
        pos = line.find("->");
        if (pos != std::string::npos) return pos;
        return std::string::npos;
    };

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        rawLines.push_back(line);

        size_t pos = extractSep(line);
        if (pos == std::string::npos) continue;

        std::string lhsName = trim(line.substr(0, pos));
        if (!lhsName.empty()) lhsNames.insert(lhsName);
    }

    // Collect all symbol names from RHS
    std::set<std::string> allSymbolNames;
    for (const auto& rawLine : rawLines) {
        size_t pos = extractSep(rawLine);
        if (pos == std::string::npos) continue;

        // Skip past separator (both "::=" and "->" are 2+ chars; ::= is 3)
        size_t sepLen = (rawLine.substr(pos, 3) == "::=") ? 3 : 2;
        std::string rhsPart = trim(rawLine.substr(pos + sepLen));

        for (const auto& sym : splitTokens(rhsPart)) {
            if (sym != "|") allSymbolNames.insert(sym);
        }
    }

    // Create symbols
    std::map<std::string, SymbolPtr> symbols;
    for (const auto& name : allSymbolNames) {
        SymbolType type = lhsNames.count(name)
                            ? SymbolType::NON_TERMINAL
                            : SymbolType::TERMINAL;
        symbols[name] = grammar->getOrCreateSymbol(name, type);
    }
    // Ensure LHS symbols exist even if they never appear on the RHS
    for (const auto& name : lhsNames) {
        if (symbols.find(name) == symbols.end()) {
            symbols[name] = grammar->getOrCreateSymbol(name, SymbolType::NON_TERMINAL);
        }
    }

    // Second pass: parse productions
    bool startSymbolSet = false;
    for (const auto& rawLine : rawLines) {
        size_t pos = extractSep(rawLine);
        if (pos == std::string::npos) continue;

        size_t sepLen = (rawLine.substr(pos, 3) == "::=") ? 3 : 2;
        std::string lhsName = trim(rawLine.substr(0, pos));
        std::string rhsPart = trim(rawLine.substr(pos + sepLen));

        auto lhsSym = symbols[lhsName];

        // Split alternatives by '|'
        std::vector<std::string> alternatives;
        {
            std::istringstream ss(rhsPart);
            std::string tok, current;
            while (ss >> tok) {
                if (tok == "|") {
                    if (!current.empty()) { alternatives.push_back(current); current.clear(); }
                } else {
                    if (!current.empty()) current += ' ';
                    current += tok;
                }
            }
            if (!current.empty()) alternatives.push_back(current);
        }

        for (const auto& alt : alternatives) {
            std::vector<SymbolPtr> rhsSymbols;
            for (const auto& symName : splitTokens(alt)) {
                auto it = symbols.find(symName);
                if (it != symbols.end()) rhsSymbols.push_back(it->second);
            }
            if (!rhsSymbols.empty()) {
                grammar->addProduction(lhsSym, rhsSymbols);
            }
        }

        if (!startSymbolSet) {
            grammar->startSymbol = lhsSym;
            startSymbolSet = true;
        }
    }

    return grammar;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: lazyparser_generator <grammar_file>\n";
        return 1;
    }

    auto grammar = parseGrammarFromFile(argv[1]);
    if (!grammar) return 1;

    std::cout << "Grammar loaded: "
              << grammar->getProductionCount() << " productions, "
              << grammar->getSymbolCount()     << " symbols\n";
    std::cout << "Start symbol: " << grammar->startSymbol->name << "\n";

    LazyParser parser(grammar);
    if (!parser.buildParser()) {
        std::cerr << "Failed to build parser!\n";
        return 1;
    }

    std::cout << "Parser built successfully!\n";
    parser.printStatistics();

    // Print parse table
    auto table = parser.getParseTable();
    if (table) {
        std::cout << "\nAction Table:\n";
        for (const auto& [stateId, actions] : table->actionTable) {
            std::cout << "  State " << stateId << ":\n";
            for (const auto& [symbol, action] : actions) {
                std::cout << "    " << symbol->name << ": "
                          << action.toString() << "\n";
            }
        }

        std::cout << "\nGoto Table:\n";
        for (const auto& [stateId, gotoMap] : table->gotoTable) {
            std::cout << "  State " << stateId << ":\n";
            for (const auto& [symbol, stateNum] : gotoMap) {
                std::cout << "    " << symbol->name
                          << " -> state " << stateNum << "\n";
            }
        }
    }

    // Interactive parsing mode
    std::cout << "\nEnter tokens to parse (space-separated, 'quit' to exit):\n";
    std::string inputLine;
    while (std::getline(std::cin, inputLine)) {
        if (inputLine == "quit") break;

        std::vector<std::pair<SymbolPtr, std::any>> tokens;
        for (const auto& sym : splitTokens(inputLine)) {
            auto s = grammar->getOrCreateSymbol(sym, SymbolType::TERMINAL);
            tokens.push_back({s, std::any()});
        }

        if (!tokens.empty()) {
            auto result = parser.parse(tokens);
            std::cout << "Result: "
                      << (result.success ? "OK" : ("FAIL: " + result.error))
                      << "\n";
        }
    }

    return 0;
}
