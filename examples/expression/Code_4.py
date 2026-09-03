# LemonPP Expression Parser Example
# Parsing arithmetic expressions with function calls

# Tokens
IDENTIFIER := [a-zA-Z_][a-zA-Z0-9_]*
NUMBER := [0-9]+(.[0-9]+)?
PLUS := +
MINUS := -
MUL := *
DIV := /
LPAREN := (
RPAREN := )
COMMA := ,

# Grammar rules
E ::= E + T | E - T | T
T ::= T * F | T / F | F
F ::= NUM | IDENTIFIER | '(' E ')'

# Start symbol
start: E