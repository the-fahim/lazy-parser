# LemonPP Expression Parser - with operator precedence
# Demonstrating precedence and associativity

%{
#include <stdio.h>
#include <math.h>
%}

%%

/* Operator precedence (from lowest to highest) */
E: E '+' T   { printf("Addition: %d + %d = %d\n", $1, $3, $1 + $3); $$ = $1 + $3; }
   | E '-' T  { printf("Subtraction: %d - %d = %d\n", $1, $3, $1 - $3); $$ = $1 - $3; }
   | T        { $$ = $1; }

T: T '*' F   { printf("Multiplication: %d * %d = %d\n", $1, $3, $1 * $3); $$ = $1 * $3; }
   | T '/' F  { printf("Division: %d / %d = %d\n", $1, $3, (int)($1 / $3)); $$ = (int)($1 / $3); }
   | F        { $$ = $1; }

F: '(' E ')' { $$ = $2; }
   | NUMBER   { $$ = atoi(yytext); }
   | '-' F    { $$ = -$2; }

%%

int yylex(void) {
    int c;
    while ((c = getchar()) == ' ' || c == '\\t');
    if (c == EOF) return 0;
    ungetc(c, stdin);
    
    /* Read number */
    if (isdigit(c)) {
        yylval = atof(yytext);
        return NUMBER;
    }
    
    /* Read identifier */
    if (isalpha(c)) {
        yylval = 0; /* placeholder */
        while (isalnum(c = getchar())) {}
        ungetc(c, stdin);
        return IDENTIFIER;
    }
    
    /* Single character operators */
    return c;
}

int main() {
    printf("Enter expression: ");
    yyparse();
    return 0;
}

int yyparse(void) {
    /* Simple recursive descent for demo */
    return 0;
}