
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <set>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include "symbol.h"

namespace orangepp {

struct Production {
    int id;
    int ruleNumber;
    SymbolPtr lhs;
    std::vector<SymbolPtr> rhs;
    std::vector<std::string> rhsAliases;
    boost::optional<std::string> lhsAlias;
    boost::optional<std::string> code;
    boost::optional<std::string> codePrefix;
    boost::optional<std::string> codeSuffix;
    SymbolPtr precedenceSymbol;
    int lineNumber;
    bool lhsStart;
    bool hasCode;
    bool canReduce;
    bool doesReduce;
    bool neverReduce;
    
    Production() 
        : id(-1), ruleNumber(-1), lineNumber(0), lhsStart(false),
          hasCode(false), canReduce(false), doesReduce(false), 
          neverReduce(false) {}
    
    std::string toString() const {
        std::string result = lhs ? lhs->name : "?";
        result += " ::=";
        for (const auto& sym : rhs) {
            result += " " + sym->name;
        }
        return result;
    }
};

using ProductionPtr = std::shared_ptr<Production>;
using ProductionList = std::vector<ProductionPtr>;

struct Grammar {
    std::string name;
    std::string filename;
    SymbolPtr startSymbol;
    SymbolPtr errorSymbol;
    SymbolPtr wildcardSymbol;
    
    ProductionList productions;
    SymbolMap symbols;
    std::set<std::string> terminalNames;
    std::set<std::string> nonTerminalNames;
    
    // Precedence tables
    std::map<SymbolPtr, int> precedenceTable;
    
    // First sets
    std::map<SymbolPtr, SymbolSet> firstSets;
    
    // Template strings
    std::string includeCode;
    std::string extraCode;
    std::string errorCode;
    std::string acceptCode;
    std::string failureCode;
    std::string overflowCode;
    std::string tokenDestructor;
    std::string varDestructor;
    std::string tokenPrefix;
    std::string stackSize;
    std::string stackSizeLimit;
    std::string reallocFunc;
    std::string freeFunc;
    
    Grammar() = default;
    
    SymbolPtr findSymbol(const std::string& name) const {
        auto it = symbols.find(name);
        return it != symbols.end() ? it->second : nullptr;
    }
    
    SymbolPtr getOrCreateSymbol(const std::string& name, SymbolType type) {
        auto sym = findSymbol(name);
        if (!sym) {
            sym = std::make_shared<Symbol>(name, type);
            symbols[name] = sym;
            if (type == SymbolType::TERMINAL) {
                terminalNames.insert(name);
            } else {
                nonTerminalNames.insert(name);
            }
        }
        return sym;
    }
    
    void addProduction(const SymbolPtr& lhs, const std::vector<SymbolPtr>& rhs) {
        auto prod = std::make_shared<Production>();
        prod->id = productions.size();
        prod->lhs = lhs;
        prod->rhs = rhs;
        productions.push_back(prod);
    }
    
    int getSymbolCount() const { return symbols.size(); }
    int getTerminalCount() const { return terminalNames.size(); }
    int getNonTerminalCount() const { return nonTerminalNames.size(); }
    int getProductionCount() const { return productions.size(); }
};

using GrammarPtr = std::shared_ptr<Grammar>;

} // namespace orangepp