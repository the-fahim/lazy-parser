#pragma once

#include <vector>
#include <map>
#include <set>
#include <memory>
#include "symbol.h"
#include "grammar.h"

namespace orangepp {

struct Item {
    ProductionPtr production;
    int dotPosition;
    SymbolSet lookahead;
    bool isComplete;
    
    Item() : dotPosition(0), isComplete(false) {}
    Item(const ProductionPtr& prod, int dot) 
        : production(prod), dotPosition(dot), isComplete(false) {}
    
    bool operator==(const Item& other) const {
        return production->id == other.production->id && 
               dotPosition == other.dotPosition &&
               lookahead == other.lookahead;
    }
    
    bool operator<(const Item& other) const {
        if (production->id != other.production->id) 
            return production->id < other.production->id;
        if (dotPosition != other.dotPosition) 
            return dotPosition < other.dotPosition;
        return lookahead < other.lookahead;
    }
    
    SymbolPtr getNextSymbol() const {
        if (dotPosition < production->rhs.size()) {
            return production->rhs[dotPosition];
        }
        return nullptr;
    }
    
    bool isCompleteItem() const {
        return dotPosition >= production->rhs.size();
    }
};

struct State {
    int id;
    std::set<Item> items;
    std::set<Item> basisItems;
    std::map<SymbolPtr, int> transitions;
    std::map<SymbolPtr, std::vector<int>> actions;
    int defaultReduceRule;
    bool isAutoReduce;
    
    State(int i) : id(i), defaultReduceRule(-1), isAutoReduce(false) {}
    
    bool operator<(const State& other) const {
        return id < other.id;
    }
    
    void addItem(const Item& item) {
        items.insert(item);
    }
    
    void addBasisItem(const Item& item) {
        basisItems.insert(item);
        items.insert(item);
    }
    
    bool hasItem(const Item& item) const {
        return items.find(item) != items.end();
    }
    
    std::vector<Item> getCompleteItems() const {
        std::vector<Item> complete;
        for (const auto& item : items) {
            if (item.isCompleteItem()) {
                complete.push_back(item);
            }
        }
        return complete;
    }
};

using StatePtr = std::shared_ptr<State>;
using StateList = std::vector<StatePtr>;

} // namespace orangepp

