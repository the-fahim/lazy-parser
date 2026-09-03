#pragma once

#include<set>
#include<string>
#include<vector>
#include<map>
#include<memory>

#include<boost/optional.hpp>
#include<boost/variant.hpp>

namespace orangepp{

    enum class SymbolType{
        TERMINAL,
        NON_TERMINAL,
        MULTI_TERMINAL
    };
    enum class Associativity {
        LEFT,
        RIGHT,
        NONE,
        UNKNOWN
    };

    struct Symbol {
        std::string name;
        SymbolType type;
        int index;
        
        boost::optional<int> precedence;
        Associativity associativity;
        boost::optional<std::string> dataType;
        boost::optional<std::string> destructor;
        
        bool lambda;
        int useCount;
    
    // For MULTI_TERMINAL only
    std::vector<Symbol*> subSymbols;
    
    Symbol(const std::string& n, SymbolType t)
        : name(n), type(t), index(-1), associativity(Associativity::NONE),
          lambda(false), useCount(0) {}
    
    bool operator==(const Symbol& other) const {
        return name == other.name && type == other.type;
    }
    
    bool operator<(const Symbol& other) const {
        if (type != other.type) return type < other.type;
        return name < other.name;
    }
    
    bool isTerminal() const { return type == SymbolType::TERMINAL; }
    bool isNonTerminal() const { return type == SymbolType::NON_TERMINAL; }
    bool isMultiTerminal() const { return type == SymbolType::MULTI_TERMINAL; }
    bool hasPrecedence() const { return precedence.is_initialized(); }
    };

    using SymbolPtr = std::shared_ptr<Symbol>;
    using SymbolSet = std::set<SymbolPtr>; 
    using SymbolMap = std::map<std::string, SymbolPtr>;


}

