%{
#include "lang.tab.h"
#include <stdlib.h>
#include <string.h>

void yyerror(const char* s);
%}

digit   [0-9]
id      [a-zA-Z_][a-zA-Z0-9_]*
strlit  \"([^\\\"]|\\.)*\"

%%

"=="            { return EQ; }
"!="            { return NEQ; }
"<="            { return LE; }
">="            { return GE; }

"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"print"         { return PRINT; }
"def"           { return DEF; }
"return"        { return RETURN; }

{id}            { yylval.str = strdup(yytext); return IDENT; }
{digit}+        { yylval.num = atoi(yytext); return NUMBER; }

{strlit}        {
                    int len = yyleng-2;
                    char* s = malloc(len+1);
                    strncpy(s, yytext+1, len);
                    s[len] = '\0';
                    yylval.str = s;
                    return STRING;
                }

[ \t\r\n]+      { /* skip whitespace */ }
";"             { return ';'; }
","             { return ','; }
"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }

"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"<"             { return '<'; }
">"             { return '>'; }

.               { yyerror("Unknown character"); exit(1); }

%%

int yywrap(void) { return 1; }
