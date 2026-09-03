#pragma once

#include <vector>
#include <map>
#include <memory>
#include "symbol.h"
#include "grammar.h"
#include "state.h"

namespace orangepp {

enum class ActionType {
    SHIFT,
    REDUCE,
    ACCEPT,
    ERROR,
    SHIFT_CONFLICT,
    REDUCE_CONFLICT,
    SHIFT_RESOLVED,
    REDUCE_RESOLVED,
    SHIFT_REDUCE
};

struct Action {
    ActionType type;
    SymbolPtr symbol;
    boost::optional<int> state;
    boost::optional<ProductionPtr> production;
    int priority;
    
    Action() : type(ActionType::ERROR), priority(0) {}
    
    bool isShift() const { return type == ActionType::SHIFT; }
    bool isReduce() const { return type == ActionType::REDUCE; }
    bool isAccept() const { return type == ActionType::ACCEPT; }
    bool isError() const { return type == ActionType::ERROR; }
    
    std::string toString() const {
        switch (type) {
            case ActionType::SHIFT:
                return "shift " + std::to_string(state.get_value_or(-1));
            case ActionType::REDUCE:
                return "reduce " + (production ? (*production)->toString() : "?");
            case ActionType::ACCEPT:
                return "accept";
            case ActionType::ERROR:
                return "error";
            default:
                return "unknown";
        }
    }
};

struct ParseTable {
    // Action table: [state][symbol] -> action
    std::map<int, std::map<SymbolPtr, Action>> actionTable;
    
    // Goto table: [state][non-terminal] -> state
    std::map<int, std::map<SymbolPtr, int>> gotoTable;
    
    // Default reduce for each state
    std::map<int, ProductionPtr> defaultReduces;
    
    void addAction(int state, const SymbolPtr& symbol, const Action& action) {
        actionTable[state][symbol] = action;
    }
    
    void addGoto(int state, const SymbolPtr& symbol, int nextState) {
        gotoTable[state][symbol] = nextState;
    }
    
    boost::optional<Action> getAction(int state, const SymbolPtr& symbol) const {
        auto stateIt = actionTable.find(state);
        if (stateIt == actionTable.end()) return boost::none;
        
        auto actionIt = stateIt->second.find(symbol);
        if (actionIt == stateIt->second.end()) return boost::none;
        
        return actionIt->second;
    }
    
    boost::optional<int> getGoto(int state, const SymbolPtr& symbol) const {
        auto stateIt = gotoTable.find(state);
        if (stateIt == gotoTable.end()) return boost::none;
        
        auto gotoIt = stateIt->second.find(symbol);
        if (gotoIt == stateIt->second.end()) return boost::none;
        
        return gotoIt->second;
    }
};

using ParseTablePtr = std::shared_ptr<ParseTable>;

} // namespace orangepp


