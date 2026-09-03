
#pragma once

#include <any>
#include <vector>
#include <stack>
#include <memory>
#include <functional>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "grammar.h"
#include "state.h"
#include "action.h"

namespace orangepp {

struct ParseResult {
    bool success;
    std::string error;
    std::any value;
    
    ParseResult(bool s = true) : success(s) {}
    ParseResult(bool s, const std::string& e) : success(s), error(e) {}
};

class OrangePPParser {
private:
    GrammarPtr grammar;
    ParseTablePtr parseTable;
    StateList states;
    
    std::map<SymbolPtr, SymbolSet> firstSets;
    std::map<SymbolPtr, bool> lambdaCache;
    
    // Semantic action callbacks
    using SemanticAction = std::function<std::any(const std::vector<std::any>&)>;
    std::map<int, SemanticAction> semanticActions;
    
public:
    OrangePPParser();
    explicit OrangePPParser(const GrammarPtr& g);
    
    // Grammar management
    void setGrammar(const GrammarPtr& g);
    GrammarPtr getGrammar() const { return grammar; }
    
    // Parser construction
    bool buildParser();
    bool buildParser(const std::string& grammarFile);
    
    // Parse functions
    ParseResult parse(const std::vector<SymbolPtr>& tokens);
    ParseResult parse(const std::vector<std::pair<SymbolPtr, std::any>>& tokens);
    
    // Semantic actions
    void setSemanticAction(int productionId, SemanticAction action);
    void setSemanticAction(const ProductionPtr& prod, SemanticAction action);
    
    // Utilities
    void enableTracing(bool enable = true);
    void setTraceFile(FILE* file);
    void printStatistics() const;
    
    // Accessors
    const ParseTablePtr getParseTable() const { return parseTable; }
    const StateList& getStates() const { return states; }
    
private:
    bool computeFirstSets();
    bool computeLambda();
    std::set<Item> closure(const std::set<Item>& items);
    std::set<SymbolPtr> computeLookahead(const Item& item);
    bool buildStates();
    bool buildParseTable();
    void resolveConflicts();
    std::set<Item> addBasisItems();
    StatePtr findOrCreateState(const std::set<Item>& items);
    
    // Internal parsing
    struct ParseStackItem {
        int state;
        SymbolPtr symbol;
        std::any value;
    };
    
    ParseResult parseInternal(const std::vector<std::pair<SymbolPtr, std::any>>& tokens);
    bool executeSemanticAction(const ProductionPtr& prod, const std::vector<std::any>& values, std::any& result);
    
    // Tracing
    bool traceEnabled;
    FILE* traceFile;
    void trace(const std::string& message);
    
    // Error recovery
    // bool errorRecovery(std::stack<ParseStackItem>& stateStack, 
    //                   int& inputPos,
    //                   std::vector<std::pair<SymbolPtr, std::any>>& tokens);

    bool errorRecovery(std::stack<ParseStackItem>& stateStack,
                   size_t& inputPos,
                   std::vector<std::pair<SymbolPtr, std::any>>& tokens);
};

using ParserPtr = std::shared_ptr<OrangePPParser>;

} // namespace OrangePP