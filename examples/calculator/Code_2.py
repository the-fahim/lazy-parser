# LemonPP Calculator Example - with operator precedence
# Demonstrating precedence support in the parser

%%
%start { E }

%%

E: E '+' T   { $$ = $1 + $3; }
   | E '-' T   { $$ = $1 - $3; }
   | T         { $$ = $1; }

T: T '*' F   { $$ = $1 * $3; }
   | T '/' F   { $$ = $1 / $3; }
   | F         { $$ = $1; }

F: '(' E ')'  { $$ = $2; }
   | NUMBER    { $$ = $1; }

%%