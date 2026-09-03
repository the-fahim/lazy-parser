#include "orangecpp/parser.h"
#include "orangecpp/grammar.h"
#include "orangecpp/symbol.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <any>
#include <memory>
#include <set>
#include <algorithm>

using namespace orangepp;

GrammarPtr parseGrammarFromFile(const std::string& filename) {
    auto grammar = std::make_shared<Grammar>();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open grammar file: " << filename << std::endl;
        return nullptr;
    }
    
    // First pass: collect all symbol names and determine which are non-terminals
    // (those appearing on LHS)
    std::set<std::string> lhsNames;
    std::vector<std::string> rawLines;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        rawLines.push_back(line);
        // Extract LHS name (before ::= or -> or :)
        size_t pos = line.find("::=");
        if (pos == std::string::npos) {
            pos = line.find("->");
            if (pos == std::string::npos) {
                pos = line.find(":");
                if (pos == std::string::npos) continue;
            }
        }
        std::string lhs = line.substr(0, pos);
        // Trim
        while (!lhs.empty() && (lhs.back() == ' ' || lhs.back() == '\t'))
            lhs.pop_back();
        if (!lhs.empty()) lhsNames.insert(lhs);
    }
    
    // Second pass: parse productions
    file.clear();
    file.seekg(0, std::ios::beg);
    
    std::set<std::string> nonTerminalNames = lhsNames;
    
    // First, identify all symbol names from all productions
    std::set<std::string> allSymbolNames;
    for (const auto& rawLine : rawLines) {
        size_t pos = rawLine.find("::=");
        if (pos == std::string::npos) {
            pos = rawLine.find("->");
            if (pos == std::string::npos) {
                pos = rawLine.find(":");
                if (pos == std::string::npos) continue;
            }
        }
        std::string rhsPart = rawLine.substr(pos + 3);
        std::istringstream iss(rhsPart);
        std::string sym;
        while (iss >> sym) {
            allSymbolNames.insert(sym);
        }
    }
    
    // Non-terminals are those explicitly on LHS; rest are terminals
    // But we also ensure all LHS names are non-terminals
    std::set<std::string> terminalNames;
    for (const auto& name : allSymbolNames) {
        if (nonTerminalNames.count(name)) {
            // It's a non-terminal
        } else {
            terminalNames.insert(name);
        }
    }
    
    // Re-open and parse with proper symbol types
    // Actually, let's just re-use the file we already read
    // We need to create symbols with proper types
    
    // Third pass: create symbols and parse productions
    // We need to re-read the file and create symbols properly
    
    // Close and reopen
    file.clear();
    file.seekg(0, std::ios::beg);
    
    // First create all symbols
    std::map<std::string, SymbolPtr> symbols;
    
    for (const auto& name : allSymbolNames) {
        SymbolType type = (nonTerminalNames.count(name) ? SymbolType::NON_TERMINAL : SymbolType::TERMINAL);
        symbols[name] = grammar->getOrCreateSymbol(name, type);
    }
    
    // Second pass: parse productions
    // Skip first line handling for start symbol
    bool startSymbolSet = false;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find("::=");
        if (pos == std::string::npos) {
            pos = line.find("->");
            if (pos == std::string::npos) {
                pos = line.find(":");
                if (pos == std::string::npos) continue;
            }
        }
        
        std::string lhsName = line.substr(0, pos);
        std::string rhsPart = line.substr(pos + 3);
        
        // Trim
        while (!lhsName.empty() && (lhsName.back() == ' ' || lhsName.back() == '\t'))
            lhsName.pop_back();
        while (!rhsPart.empty() && (rhsPart.front() == ' ' || rhsPart.front() == '\t'))
            rhsPart.erase(0, 1);
        
        auto lhs = symbols[lhsName];
        
        // Parse RHS alternatives separated by |
        std::vector<std::string> alternatives;
        size_t altPos = 0;
        while ((altPos = rhsPart.find('|')) != std::string::npos) {
            alternatives.push_back(rhsPart.substr(0, altPos));
            rhsPart.erase(0, altPos + 1);
        }
        alternatives.push_back(rhsPart);
        
        // Add each alternative as a production
        for (const auto& alt : alternatives) {
            std::vector<SymbolPtr> rhsSymbols;
            std::istringstream iss(alt);
            std::string symName;
            while (iss >> symName) {
                auto it = symbols.find(symName);
                if (it != symbols.end()) {
                    rhsSymbols.push_back(it->second);
                }
            }
            if (!rhsSymbols.empty()) {
                grammar->addProduction(lhs, rhsSymbols);
            }
        }
        
        // First production's LHS becomes the start symbol
        if (!startSymbolSet) {
            grammar->startSymbol = lhs;
            startSymbolSet = true;
        }
    }
    
    return grammar;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: lemonpp_generator <grammar_file>" << std::endl;
        return 1;
    }
    
    auto grammar = parseGrammarFromFile(argv[1]);
    if (!grammar) return 1;
    
    std::cout << "Grammar loaded: " << grammar->getProductionCount() 
              << " productions, " << grammar->getSymbolCount() << " symbols" << std::endl;
    std::cout << "Start symbol: " << grammar->startSymbol->name << std::endl;
    
    // Build the parser
    OrangePPParser parser(grammar);
    if (!parser.buildParser()) {
        std::cerr << "Failed to build parser!" << std::endl;
        return 1;
    }
    
    std::cout << "Parser built successfully!" << std::endl;
    parser.printStatistics();
    
    // Print parse table
    auto table = parser.getParseTable();
    if (table) {
        std::cout << "\nParse Table:" << std::endl;
        for (const auto& [stateId, actions] : table->actionTable) {
            std::cout << "State " << stateId << ":" << std::endl;
            for (const auto& [symbol, action] : actions) {
                std::cout << "  " << symbol->name << ": " << action.toString() << std::endl;
            }
        }
        
        std::cout << "\nGoto Table:" << std::endl;
        for (const auto& [stateId, gotoTable] : table->gotoTable) {
            std::cout << "State " << stateId << ":" << std::endl;
            for (const auto& [symbol, stateNum] : gotoTable) {
                std::cout << "  " << symbol->name << " -> state " << stateNum << std::endl;
            }
        }
    }
    
    // Test parsing interactively
    std::cout << "\nEnter tokens to parse (type 'quit' to exit):" << std::endl;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit") break;
        
        std::istringstream iss(line);
        std::string sym;
        std::vector<std::pair<SymbolPtr, std::any>> tokens;
        
        while (iss >> sym) {
            auto s = grammar->getOrCreateSymbol(sym, SymbolType::TERMINAL);
            tokens.push_back({s, std::any()});
        }
        
        if (!tokens.empty()) {
            auto result = parser.parse(tokens);
            std::cout << "Result: " << (result.success ? "OK" : ("FAIL: " + result.error)) << std::endl;
        }
    }
    
    return 0;
}