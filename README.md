# Lazy Parser - A LR(1) Parser Generator (Like SQLite's Lemon)

Lazy Parser is a C++17 implementation of an LR(1) parser generator, inspired by SQLite's `lemon` tool. It can parse a grammar file and generate a deterministic LR(1) parser with parse tables, or use a pre-built parser to parse input tokens.
Lazy Parser is referred as LemmonPP.

## Project Structure

```
orange/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   └── orangecpp/          # C++ source headers
│       ├── action.h        # Action types (SHIFT/REDUCE/ACCEPT)
│       ├── grammar.h       # Grammar structure (Productions, Symbols)
│       ├── parser.h        # OrangePPParser class
│       ├── state.h         # LR(1) States and Items
│       ├── symbol.h       # Terminal/Non-terminal Symbols
│       └── utils.h         # Utility definitions
├── src/                    # Source files
│   ├── parser.cpp          # Main parser implementation
│   ├── grammar.cpp         # Grammar processing
│   ├── symbol.cpp          # Symbol management
│   ├── state.cpp           # State machine
│   └── action.cpp          # Action handling
├── tools/                  # Generator tool
│   └── lemonpp_generator.cpp  # Grammar file parser
├── examples/               # Example grammars (calculator, expression)
├── tests/                  # Test suites
│   ├── test_parser.cpp     # Parser functionality tests
│   ├── test_grammar.cpp    # Grammar structure tests
│   └── test_utils.cpp      # Utility tests
├── build/                  # Build directory (generated)
├── bin/                    # Built executables
│   ├── lemonpp             # Main parser executable
│   ├── lemonpp_generator   # Grammar-to-parser generator
│   ├── test_parser         # Unit tests
│   ├── test_grammar        # Grammar tests
│   └── test_utils          # Utility tests
└── CMakeLists.txt          # Tests examples config
```

## Features

### Parser Engine (`lemonpp_lib`)
- **LR(1) item computation**: Items with lookahead sets
- **FIRST set computation**: With nullable (lambda) non-terminal detection
- **State machine construction**: Closure and goto operations
- **Parse table**: Action and goto tables
- **Shift/Reduce conflict resolution**: Precedence-based resolution
- **Panic mode error recovery**: Skip invalid tokens until valid state
- **Semantic actions**: Function callbacks on rule reduction

### Main Executable (`lemonpp`)
- Build a parser from a Grammar object
- Parse token sequences and report success/failure
- Print parser statistics (states, productions, symbols)
- Enable tracing of parse actions

### Generator Tool (`lemonpp_generator`)
- Read grammar files in `lemon` format:
  ```
  E ::= E + T | T
  T ::= NUM
  ```
- Automatically classify non-terminals (LHS) vs terminals
- Build the LR(1) parser
- Display parse table (actions & goto transitions)
- Interactive token parsing session

## Building

### Requirements
- C++17 compatible compiler (GCC 16+, Clang)
- Boost 1.92 (filesystem, system)

### Compile from Source
```bash
cd orange
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
make -j$(nproc)
```

### Build Options
- `BUILD_TESTS` (ON) - Build test executables
- `BUILD_EXAMPLES` (ON) - Build example programs
- `ENABLE_COVERAGE` (OFF) - Enable GCC coverage reporting

## Usage

### 1. Using the Main Executable
```bash
./bin/lemonpp
```
- Builds a default expression grammar
- Parses "NUM", "NUM + NUM", "NUM + NUM + NUM"
- Prints parser statistics

### 2. Using the Generator Tool
```bash
./bin/lemonpp_generator grammar.txt
```
Reads a grammar file and shows the generated parse table.

**grammar.txt example:**
```
E ::= E + T | T
T ::= NUM
```

Usage:
```bash
./bin/lemonpp_generator grammar.txt
# Then type tokens like: NUM + NUM
# Type 'quit' to exit
```

### 3. Using the Parser Library
```cpp
#include "orangecpp/parser.h"
#include "orangecpp/grammar.h"

auto grammar = std::make_shared<orangepp::Grammar>();
auto expr = grammar->getOrCreateSymbol("E", orangepp::SymbolType::NON_TERMINAL);
auto num = grammar->getOrCreateSymbol("NUM", orangepp::SymbolType::TERMINAL);
auto plus = grammar->getOrCreateSymbol("+", orangepp::SymbolType::TERMINAL);

// Add productions
grammar->addProduction(expr, {expr, plus, num});  // E -> E + T
grammar->addProduction(expr, {num});              // E -> T
grammar->startSymbol = expr;

// Build parser
orangepp::OrangePPParser parser(grammar);
parser.buildParser();

// Parse tokens
std::vector<std::pair<orangepp::SymbolPtr, std::any>> tokens = {
    {num, std::any()}, {plus, std::any()}, {num, std::any()}
};
auto result = parser.parse(tokens);
if (result.success) {
    std::cout << "Parsed successfully!" << std::endl;
}
```

### 4. Running Tests
```bash
./bin/test_parser    # Tests parser functionality
./bin/test_grammar   # Tests grammar structure
./bin/test_utils     # Utility module availability
```

## Grammar Format

The generator accepts grammar files with this syntax:

```
% Start symbol is the first LHS encountered

E ::= E + T | T          # Production 1: E -> E + T, Production 2: E -> T
T ::= NUM | id           # Production 3: T -> NUM, Production 4: T -> id
```

- `::=` separates LHS from RHS
- `|` separates alternative RHS sequences
- First symbol on any LHS becomes the start symbol
- Symbols on LHS are non-terminals; others are terminals

## Grammar Example: Expression Calculator

```
E ::= E + T | T
T ::= NUM
```

This grammar parses:
- `NUM` → OK
- `NUM + NUM` → OK 
- `NUM + NUM + NUM` → OK

## Implementation Details

### LR(1) Algorithm
1. **FIRST Sets**: Compute terminal sets for each non-terminal
2. **Lambda Detection**: Identify nullable non-terminals
3. **Item Construction**: Items represent "how much of a production has been parsed"
4. **Closure**: Add items for non-terminals after the dot
5. **Goto**: Transition state by advancing the dot over a symbol
6. **Parse Table**: Shift actions on terminals, goto on non-terminals, reduce on complete items

### Data Structures
- `Symbol`: String name + Type (TERMINAL/NON_TERMINAL) + optional precedence
- `Production`: LHS + RHS vector + semantic action code
- `Item`: Production + dot position + lookahead set
- `State`: Set of items + transitions + actions
- `ParseTable`: actionTable[state][symbol] → action, gotoTable[state][non-terminal] → state

### Conflict Resolution
Shift/reduce conflicts are resolved using operator precedence:
- Each symbol can have an optional precedence value
- Higher precedence wins
- Default: prefer shift over reduce
<tool_call>
<function=bash>
<parameter=command>
cd /run/media/fahim/Anderson/PROJECTS/Arch-Family/The_World-Of-Database-internals/The-Implementations/orange && cat README.md | head -5
