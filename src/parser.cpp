#include "lazyparser/parser.h"

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

#include <iostream>
#include <print>
#include <algorithm>

namespace lazyparser {

// ---------------------------------------------------------------------------
// Constructors / setGrammar
// ---------------------------------------------------------------------------

LazyParser::LazyParser()
    : traceEnabled(false), traceFile(stderr) {}

LazyParser::LazyParser(const GrammarPtr& g)
    : grammar(g), traceEnabled(false), traceFile(stderr) {}

void LazyParser::setGrammar(const GrammarPtr& g) {
    grammar    = g;
    states.clear();
    parseTable = nullptr;
}

// ---------------------------------------------------------------------------
// buildParser
// ---------------------------------------------------------------------------

bool LazyParser::buildParser() {
    if (!grammar) return false;

    if (!computeFirstSets()) return false;
    if (!buildStates())      return false;
    if (!buildParseTable())  return false;

    return true;
}

bool LazyParser::buildParser(const std::string& /*grammarFile*/) {
    // Full grammar-file parsing is handled by lazyparser_generator.
    // This overload is a hook for future Spirit-based parsing.
    return false;
}

// ---------------------------------------------------------------------------
// FIRST sets
// ---------------------------------------------------------------------------

bool LazyParser::computeFirstSets() {
    // Initialise FIRST sets for every symbol
    for (const auto& [name, sym] : grammar->symbols) {
        firstSets[sym] = SymbolSet();
        if (sym->type == SymbolType::TERMINAL) {
            firstSets[sym].insert(sym);
        }
    }

    if (!computeLambda()) return false;

    // Iterative fixed-point computation
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& prod : grammar->productions) {
            const auto& lhs      = prod->lhs;
            bool        allNullable = true;

            for (const auto& sym : prod->rhs) {
                if (sym->type == SymbolType::TERMINAL) {
                    if (firstSets[lhs].insert(sym).second) changed = true;
                    allNullable = false;
                    break;
                } else {
                    // Union FIRST(sym) into FIRST(lhs)
                    for (const auto& fs : firstSets[sym]) {
                        if (firstSets[lhs].insert(fs).second) changed = true;
                    }
                    if (!lambdaCache[sym]) {
                        allNullable = false;
                        break;
                    }
                }
            }

            (void)allNullable; // epsilon handling is implicit via lambdaCache
        }
    }

    return true;
}

bool LazyParser::computeLambda() {
    // Initialise all non-terminals as non-nullable
    for (const auto& [name, sym] : grammar->symbols) {
        if (sym->type == SymbolType::NON_TERMINAL) {
            lambdaCache[sym] = false;
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& prod : grammar->productions) {
            if (lambdaCache[prod->lhs]) continue;

            // A production is nullable iff every RHS symbol is nullable
            bool allNullable = std::all_of(
                prod->rhs.begin(), prod->rhs.end(),
                [this](const SymbolPtr& sym) -> bool {
                    if (sym->type == SymbolType::TERMINAL) return false;
                    auto it = lambdaCache.find(sym);
                    return it != lambdaCache.end() && it->second;
                });

            if (allNullable) {
                lambdaCache[prod->lhs] = true;
                changed = true;
            }
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// State construction
// ---------------------------------------------------------------------------

bool LazyParser::buildStates() {
    if (!grammar->startSymbol) return false;

    // Create augmented production: S' -> start
    auto augStart = grammar->getOrCreateSymbol("S'", SymbolType::NON_TERMINAL);
    grammar->addProduction(augStart, {grammar->startSymbol});

    // Initial item: S' -> . start  {$}
    Item initialItem(grammar->productions.back(), 0);
    initialItem.lookahead.insert(
        grammar->getOrCreateSymbol("$", SymbolType::TERMINAL));

    std::set<Item> initialItems = {initialItem};
    auto closureItems = closure(initialItems);

    auto initialState         = std::make_shared<State>(0);
    initialState->basisItems  = initialItems;
    initialState->items       = closureItems;
    states.push_back(initialState);

    size_t index = 0;
    while (index < states.size()) {
        auto currentState = states[index++];

        // Group items by the symbol immediately after the dot
        std::map<SymbolPtr, std::set<Item>> groupedItems;
        for (const auto& item : currentState->items) {
            auto nextSym = item.getNextSymbol();
            if (nextSym) {
                Item newItem(item.production, item.dotPosition + 1);
                newItem.lookahead = item.lookahead;
                groupedItems[nextSym].insert(newItem);
            }
        }

        for (const auto& [symbol, items] : groupedItems) {
            auto newState = findOrCreateState(closure(items));

            if (symbol->type == SymbolType::TERMINAL) {
                currentState->actions[symbol].push_back(newState->id);
            } else {
                currentState->transitions[symbol] = newState->id;
            }
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Closure / lookahead
// ---------------------------------------------------------------------------

std::set<Item> LazyParser::closure(const std::set<Item>& items) {
    std::set<Item> closureSet = items;
    bool added = true;

    while (added) {
        added = false;
        std::set<Item> newItems;

        for (const auto& item : closureSet) {
            auto nextSym = item.getNextSymbol();
            if (!nextSym || nextSym->type != SymbolType::NON_TERMINAL) continue;

            auto lookahead = computeLookahead(item);

            for (const auto& prod : grammar->productions) {
                if (prod->lhs == nextSym) {
                    Item newItem(prod, 0);
                    newItem.lookahead = lookahead;
                    if (closureSet.find(newItem) == closureSet.end()) {
                        newItems.insert(newItem);
                    }
                }
            }
        }

        for (const auto& item : newItems) {
            if (closureSet.insert(item).second) added = true;
        }
    }

    return closureSet;
}

std::set<SymbolPtr> LazyParser::computeLookahead(const Item& item) {
    std::set<SymbolPtr> lookahead = item.lookahead;

    // Collect beta = symbols strictly after the dot
    std::vector<SymbolPtr> beta;
    for (int i = item.dotPosition + 1;
         i < static_cast<int>(item.production->rhs.size()); ++i) {
        beta.push_back(item.production->rhs[i]);
    }

    if (beta.empty()) return lookahead;

    bool allNullable = true;
    for (const auto& sym : beta) {
        if (sym->type == SymbolType::TERMINAL) {
            lookahead.insert(sym);
            allNullable = false;
            break;
        } else {
            boost::range::copy(firstSets[sym],
                               std::inserter(lookahead, lookahead.end()));
            if (!lambdaCache[sym]) {
                allNullable = false;
                break;
            }
        }
    }

    (void)allNullable;
    return lookahead;
}

// ---------------------------------------------------------------------------
// Parse table construction
// ---------------------------------------------------------------------------

bool LazyParser::buildParseTable() {
    parseTable = std::make_shared<ParseTable>();

    auto dollarSym = grammar->getOrCreateSymbol("$", SymbolType::TERMINAL);

    for (const auto& state : states) {
        // Shift actions
        for (const auto& [symbol, nextStates] : state->actions) {
            if (!nextStates.empty()) {
                Action action;
                action.type   = ActionType::SHIFT;
                action.symbol = symbol;
                action.state  = nextStates[0];
                parseTable->addAction(state->id, symbol, action);
            }
        }

        // Goto transitions
        for (const auto& [symbol, nextState] : state->transitions) {
            parseTable->addGoto(state->id, symbol, nextState);
        }

        // Reduce / accept actions for complete items
        for (const auto& item : state->getCompleteItems()) {
            if (item.production->lhs->name == "S'" &&
                item.production->lhs->type == SymbolType::NON_TERMINAL) {
                // Accept
                Action action;
                action.type   = ActionType::ACCEPT;
                action.symbol = dollarSym;
                parseTable->addAction(state->id, dollarSym, action);
            } else {
                for (const auto& look : item.lookahead) {
                    Action action;
                    action.type       = ActionType::REDUCE;
                    action.symbol     = look;
                    action.production = item.production;
                    parseTable->addAction(state->id, look, action);
                }
            }
        }
    }

    resolveConflicts();
    return true;
}

void LazyParser::resolveConflicts() {
    for (auto& [stateId, actions] : parseTable->actionTable) {
        // Collect per-symbol action lists for conflict detection
        std::map<SymbolPtr, std::vector<Action>> grouped;
        for (const auto& [symbol, action] : actions) {
            grouped[symbol].push_back(action);
        }

        for (auto& [symbol, actionList] : grouped) {
            if (actionList.size() <= 1) continue;

            bool hasShift  = std::any_of(actionList.begin(), actionList.end(),
                [](const Action& a){ return a.type == ActionType::SHIFT; });
            bool hasReduce = std::any_of(actionList.begin(), actionList.end(),
                [](const Action& a){ return a.type == ActionType::REDUCE; });

            if (hasShift && hasReduce) {
                if (symbol->hasPrecedence()) {
                    // Select highest-precedence action
                    int    maxPrec = -1;
                    Action selected;

                    for (const auto& action : actionList) {
                        int prec = 0;
                        if (action.type == ActionType::SHIFT) {
                            prec = symbol->precedence.value_or(0);
                        } else if (action.type == ActionType::REDUCE) {
                            if (action.production &&
                                (*action.production)->precedenceSymbol) {
                                prec = (*action.production)
                                           ->precedenceSymbol->precedence.value_or(0);
                            }
                        }
                        if (prec > maxPrec) { maxPrec = prec; selected = action; }
                    }

                    actions[symbol] = selected;
                } else {
                    // Default: prefer shift
                    for (const auto& action : actionList) {
                        if (action.type == ActionType::SHIFT) {
                            actions[symbol] = action;
                            break;
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// State lookup
// ---------------------------------------------------------------------------

std::set<Item> LazyParser::addBasisItems() {
    std::set<Item> basisItems;
    if (grammar->startSymbol) {
        for (const auto& prod : grammar->productions) {
            if (prod->lhs == grammar->startSymbol) {
                basisItems.insert(Item(prod, 0));
            }
        }
    }
    return basisItems;
}

StatePtr LazyParser::findOrCreateState(const std::set<Item>& items) {
    for (const auto& state : states) {
        if (state->items == items) return state;
    }
    auto newState        = std::make_shared<State>(static_cast<int>(states.size()));
    newState->items      = items;
    newState->basisItems = items;
    states.push_back(newState);
    return newState;
}

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

ParseResult LazyParser::parse(const std::vector<SymbolPtr>& tokens) {
    std::vector<std::pair<SymbolPtr, std::any>> tokenValues;
    tokenValues.reserve(tokens.size());
    for (const auto& token : tokens) {
        tokenValues.push_back({token, std::any()});
    }
    return parseInternal(tokenValues);
}

ParseResult LazyParser::parse(
    const std::vector<std::pair<SymbolPtr, std::any>>& tokens) {
    return parseInternal(tokens);
}

ParseResult LazyParser::parseInternal(
    const std::vector<std::pair<SymbolPtr, std::any>>& tokens) {

    if (!parseTable) return ParseResult(false, "Parse table not built");

    std::stack<ParseStackItem> stateStack;
    stateStack.push({0, nullptr, std::any()});

    auto augTokens = tokens;
    augTokens.push_back(
        {grammar->getOrCreateSymbol("$", SymbolType::TERMINAL), std::any()});

    size_t inputPos = 0;

    while (true) {
        int       currentState  = stateStack.top().state;
        auto      currentSymbol = augTokens[inputPos].first;

        auto actionOpt = parseTable->getAction(currentState, currentSymbol);

        if (!actionOpt) {
            if (!errorRecovery(stateStack, inputPos, augTokens)) {
                return ParseResult(false,
                    "Syntax error at position " + std::to_string(inputPos));
            }
            continue;
        }

        Action action = *actionOpt;

        if (action.isShift()) {
            ParseStackItem item;
            item.state  = action.state.value_or(0);
            item.symbol = currentSymbol;
            item.value  = augTokens[inputPos].second;
            stateStack.push(item);
            ++inputPos;

            if (traceEnabled) {
                trace(boost::str(boost::format("Shift: %1% -> state %2%")
                    % currentSymbol->name % item.state));
            }

        } else if (action.isReduce()) {
            auto prod = action.production.value_or(nullptr);
            if (!prod) return ParseResult(false, "Invalid reduce action");

            std::vector<std::any> rhsValues;
            rhsValues.reserve(prod->rhs.size());
            for (size_t i = 0; i < prod->rhs.size(); ++i) {
                if (!stateStack.empty()) {
                    rhsValues.push_back(stateStack.top().value);
                    stateStack.pop();
                }
            }
            std::reverse(rhsValues.begin(), rhsValues.end());

            std::any result;
            if (!executeSemanticAction(prod, rhsValues, result)) {
                return ParseResult(false,
                    "Semantic action failed for rule: " + prod->toString());
            }

            int gotoState = parseTable->getGoto(stateStack.top().state, prod->lhs)
                                .value_or(-1);
            if (gotoState < 0) {
                return ParseResult(false,
                    "No goto state for: " + prod->lhs->name);
            }

            ParseStackItem item;
            item.state  = gotoState;
            item.symbol = prod->lhs;
            item.value  = result;
            stateStack.push(item);

            if (traceEnabled) {
                trace(boost::str(boost::format("Reduce: %1% -> state %2%")
                    % prod->toString() % gotoState));
            }

        } else if (action.isAccept()) {
            if (traceEnabled) trace("Accept!");
            return ParseResult(true);

        } else {
            return ParseResult(false, "Invalid action");
        }
    }
}

// ---------------------------------------------------------------------------
// Semantic actions
// ---------------------------------------------------------------------------

bool LazyParser::executeSemanticAction(const ProductionPtr& prod,
                                       const std::vector<std::any>& values,
                                       std::any& result) {
    auto it = semanticActions.find(prod->id);
    if (it != semanticActions.end()) {
        result = it->second(values);
        return true;
    }
    if (!values.empty()) result = values[0];
    return true;
}

void LazyParser::setSemanticAction(int productionId, SemanticAction action) {
    semanticActions[productionId] = action;
}

void LazyParser::setSemanticAction(const ProductionPtr& prod, SemanticAction action) {
    semanticActions[prod->id] = action;
}

// ---------------------------------------------------------------------------
// Error recovery (panic mode)
// ---------------------------------------------------------------------------

bool LazyParser::errorRecovery(
    std::stack<ParseStackItem>& stateStack,
    size_t& inputPos,
    std::vector<std::pair<SymbolPtr, std::any>>& tokens) {

    if (traceEnabled) {
        trace("Error recovery: skipping token at position " +
              std::to_string(inputPos));
    }

    while (inputPos < tokens.size()) {
        ++inputPos;
        if (!stateStack.empty() && inputPos < tokens.size()) {
            int currentState = stateStack.top().state;
            if (parseTable->getAction(currentState, tokens[inputPos].first)) {
                return true;
            }
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Tracing / statistics
// ---------------------------------------------------------------------------

void LazyParser::enableTracing(bool enable) {
    traceEnabled = enable;
}

void LazyParser::setTraceFile(FILE* file) {
    traceFile = file;
}

void LazyParser::trace(const std::string& message) {
    if (traceEnabled && traceFile) {
        std::print(traceFile, "LazyParser: {}\n", message);
    }
}

void LazyParser::printStatistics() const {
    std::print("=== LazyParser Statistics ===\n");
    std::print("States:         {}\n", states.size());
    std::print("Productions:    {}\n",
               grammar ? grammar->getProductionCount() : 0);
    std::print("Symbols:        {}\n",
               grammar ? grammar->getSymbolCount() : 0);
    std::print("Terminals:      {}\n",
               grammar ? grammar->getTerminalCount() : 0);
    std::print("Non-terminals:  {}\n",
               grammar ? grammar->getNonTerminalCount() : 0);

    if (parseTable) {
        size_t actionCount = 0;
        for (const auto& [state, actions] : parseTable->actionTable) {
            actionCount += actions.size();
        }
        std::print("Actions:        {}\n", actionCount);
    }
}

} // namespace lazyparser