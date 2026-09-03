# LemonPP Calculator Example
# A simple expression calculator grammar

# Tokens
NUM := number
PLUS := +
MINUS := -
MUL := *
DIV := /
LPAREN := (
RPAREN := )

# Grammar rules
E ::= E + T | E - T | T
T ::= T * F | T / F | F
F ::= NUM | ( E )

# Start symbol
start: E