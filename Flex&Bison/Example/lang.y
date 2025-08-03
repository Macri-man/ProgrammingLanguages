%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SCOPE 100
#define MAX_PARAMS 10

%define parse.trace

typedef enum { TYPE_INT, TYPE_STRING } ValueType;

typedef struct {
    ValueType type;
    union {
        int i;
        char* s;
    } val;
} Value;

typedef struct Var {
    char* name;
    Value value;
    struct Var* next;
} Var;

typedef struct Func {
    char* name;
    char** params;
    int param_count;
    struct Stmt* body;
    struct Func* next;
} Func;

typedef struct Expr {
    enum { EXPR_NUM, EXPR_STR, EXPR_VAR, EXPR_BINOP, EXPR_NEG, EXPR_CALL } type;
    ValueType value_type;
    union {
        int num;
        char* str;
        char* id;
        struct { char op; struct Expr* left; struct Expr* right; } binop;
        struct { char* id; struct Expr** args; int argc; } call;
        struct Expr* subexpr;
    } data;
    struct Expr* next;  // For argument lists
} Expr;

typedef struct Stmt {
    enum { STMT_EXPR, STMT_ASSIGN, STMT_PRINT, STMT_IF, STMT_WHILE, STMT_BLOCK, STMT_RETURN } type;
    char* id;
    Expr* expr1;
    struct Stmt* block1;
    struct Stmt* block2;
    struct Stmt* next;
} Stmt;

Var* scopes[MAX_SCOPE];
int scope_level = 0;

Func* funcs = NULL;

Value ret_val;
bool returning = false;

// Function declarations
void push_scope();
void pop_scope();
Var* find_var(const char* name);
void set_var(const char* name, Value val);
Func* find_func(const char* name);
void define_func(const char* name, char** params, int param_count, Stmt* body);
Value eval_expr(Expr* e);
void execute_stmt(Stmt* s);
Value eval_binop(char op, Value left, Value right);

// Scope management
void push_scope() {
    if (scope_level + 1 >= MAX_SCOPE) {
        fprintf(stderr, "Scope overflow\n");
        exit(1);
    }
    scope_level++;
    scopes[scope_level] = NULL;
}

void pop_scope() {
    Var* v = scopes[scope_level];
    while (v) {
        Var* tmp = v;
        v = v->next;
        free(tmp->name);
        if (tmp->value.type == TYPE_STRING) free(tmp->value.val.s);
        free(tmp);
    }
    scopes[scope_level] = NULL;
    scope_level--;
}

Var* find_var(const char* name) {
    for (int i = scope_level; i >= 0; i--) {
        Var* v = scopes[i];
        while (v) {
            if (strcmp(v->name, name) == 0) return v;
            v = v->next;
        }
    }
    return NULL;
}

void set_var(const char* name, Value val) {
    Var* v = find_var(name);
    if (!v) {
        v = malloc(sizeof(Var));
        v->name = strdup(name);
        v->next = scopes[scope_level];
        scopes[scope_level] = v;
    } else {
        if (v->value.type == TYPE_STRING) free(v->value.val.s);
    }
    v->value = val;
}

Func* find_func(const char* name) {
    Func* f = funcs;
    while (f) {
        if (strcmp(f->name, name) == 0) return f;
        f = f->next;
    }
    return NULL;
}

void define_func(const char* name, char** params, int param_count, Stmt* body) {
    Func* f = find_func(name);
    if (!f) {
        f = malloc(sizeof(Func));
        f->name = strdup(name);
        f->params = malloc(sizeof(char*) * param_count);
        for (int i = 0; i < param_count; i++) {
            f->params[i] = strdup(params[i]);
        }
        f->param_count = param_count;
        f->body = body;
        f->next = funcs;
        funcs = f;
    }
}

// Forward declarations
Value eval_expr(Expr* e);
void execute_stmt(Stmt* s);

// Evaluate binary operations with type checking
Value eval_binop(char op, Value left, Value right) {
    if (left.type == TYPE_INT && right.type == TYPE_INT) {
        int l = left.val.i;
        int r = right.val.i;
        switch (op) {
            case '+': return (Value){TYPE_INT, .val.i = l + r};
            case '-': return (Value){TYPE_INT, .val.i = l - r};
            case '*': return (Value){TYPE_INT, .val.i = l * r};
            case '/':
                if (r == 0) {
                    fprintf(stderr, "Division by zero\n");
                    exit(1);
                }
                return (Value){TYPE_INT, .val.i = l / r};
            case '<': return (Value){TYPE_INT, .val.i = l < r};
            case '>': return (Value){TYPE_INT, .val.i = l > r};
            case '=': return (Value){TYPE_INT, .val.i = l == r}; // For EQ token
            case '!': return (Value){TYPE_INT, .val.i = l != r}; // For NEQ token
            case 'L': return (Value){TYPE_INT, .val.i = l <= r}; // LE token
            case 'G': return (Value){TYPE_INT, .val.i = l >= r}; // GE token
        }
    }
    if (op == '+' && left.type == TYPE_STRING && right.type == TYPE_STRING) {
        size_t len = strlen(left.val.s) + strlen(right.val.s) + 1;
        char* s = malloc(len);
        strcpy(s, left.val.s);
        strcat(s, right.val.s);
        return (Value){TYPE_STRING, .val.s = s};
    }
    fprintf(stderr, "Type error in binary operation\n");
    exit(1);
}

Value eval_expr(Expr* e) {
    switch (e->type) {
        case EXPR_NUM:
            return (Value){TYPE_INT, .val.i = e->data.num};
        case EXPR_STR:
            return (Value){TYPE_STRING, .val.s = strdup(e->data.str)};
        case EXPR_VAR: {
            Var* v = find_var(e->data.id);
            if (!v) {
                fprintf(stderr, "Undefined variable '%s'\n", e->data.id);
                exit(1);
            }
            return v->value;
        }
        case EXPR_BINOP: {
            Value l = eval_expr(e->data.binop.left);
            Value r = eval_expr(e->data.binop.right);
            return eval_binop(e->data.binop.op, l, r);
        }
        case EXPR_NEG: {
            Value sub = eval_expr(e->data.subexpr);
            if (sub.type != TYPE_INT) {
                fprintf(stderr, "Unary - applied to non-int\n");
                exit(1);
            }
            return (Value){TYPE_INT, .val.i = -sub.val.i};
        }
        case EXPR_CALL: {
            Func* f = find_func(e->data.call.id);
            if (!f) {
                fprintf(stderr, "Undefined function '%s'\n", e->data.call.id);
                exit(1);
            }
            if (f->param_count != e->data.call.argc) {
                fprintf(stderr, "Function '%s' expects %d args, got %d\n", f->name, f->param_count, e->data.call.argc);
                exit(1);
            }
            push_scope();
            for (int i = 0; i < f->param_count; i++) {
                Value val = eval_expr(e->data.call.args[i]);
                set_var(f->params[i], val);
            }
            returning = false;
            execute_stmt(f->body);
            pop_scope();
            if (!returning) return (Value){TYPE_INT, .val.i = 0};
            returning = false;
            return ret_val;
        }
    }
    fprintf(stderr, "Unknown expression type\n");
    exit(1);
}

void execute_stmt(Stmt* s) {
    while (s && !returning) {
        switch (s->type) {
            case STMT_EXPR:
                eval_expr(s->expr1);
                break;
            case STMT_ASSIGN: {
                Value val = eval_expr(s->expr1);
                set_var(s->id, val);
                break;
            }
            case STMT_PRINT: {
                Value val = eval_expr(s->expr1);
                if (val.type == TYPE_INT) printf("%d\n", val.val.i);
                else printf("%s\n", val.val.s);
                break;
            }
            case STMT_IF: {
                Value cond = eval_expr(s->expr1);
                bool cond_true = (cond.type == TYPE_INT && cond.val.i != 0) || (cond.type == TYPE_STRING && strlen(cond.val.s) != 0);
                if (cond_true) execute_stmt(s->block1);
                else if (s->block2) execute_stmt(s->block2);
                break;
            }
            case STMT_WHILE: {
                while (true) {
                    Value cond = eval_expr(s->expr1);
                    bool cond_true = (cond.type == TYPE_INT && cond.val.i != 0) || (cond.type == TYPE_STRING && strlen(cond.val.s) != 0);
                    if (!cond_true) break;
                    execute_stmt(s->block1);
                    if (returning) break;
                }
                break;
            }
            case STMT_BLOCK: {
                push_scope();
                execute_stmt(s->block1);
                pop_scope();
                break;
            }
            case STMT_RETURN: {
                ret_val = eval_expr(s->expr1);
                returning = true;
                break;
            }
        }
        s = s->next;
    }
}

void free_expr(Expr* e) {
    if (!e) return;
    switch (e->type) {
        case EXPR_VAR: free(e->data.id); break;
        case EXPR_STR: free(e->data.str); break;
        case EXPR_BINOP:
            free_expr(e->data.binop.left);
            free_expr(e->data.binop.right);
            break;
        case EXPR_NEG:
            free_expr(e->data.subexpr);
            break;
        case EXPR_CALL:
            free(e->data.call.id);
            for (int i=0; i<e->data.call.argc; i++) free_expr(e->data.call.args[i]);
            free(e->data.call.args);
            break;
        default: break;
    }
    free(e);
}

void free_stmt(Stmt* s) {
    while (s) {
        Stmt* next = s->next;
        if (s->id) free(s->id);
        if (s->expr1) free_expr(s->expr1);
        if (s->block1) free_stmt(s->block1);
        if (s->block2) free_stmt(s->block2);
        free(s);
        s = next;
    }
}

int yylex(void);
void yyerror(const char* s);

int param_count = 0;
char* param_list[MAX_PARAMS];

Stmt* root = NULL;

%}


%union {
    int num;
    char* str;
    struct Expr* expr;
    struct Stmt* stmt;
    char** strlist;
}

%token <num> NUMBER
%token <str> IDENT STRING
%token IF ELSE WHILE PRINT DEF RETURN

%token EQ NEQ LE GE

%type <expr> expr expr_list
%type <stmt> statement
%type <stmt> program block
%type <strlist> param_list

%left EQ NEQ
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%right UMINUS


%%

program:
      program statement  { if($2) { 
                              if (!$1) $$ = $2; 
                              else { 
                                Stmt* s = $1; 
                                while(s->next) s=s->next; 
                                s->next = $2; 
                                $$ = $1; 
                              } 
                            root = $$; }
                         }
    |                   { $$ = NULL; root = NULL; }
    ;

statement:
      expr ';'           {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_EXPR;
                            $$->expr1 = $1;
                            $$->next = NULL;
                            $$->id = NULL;
                            $$->block1 = NULL;
                            $$->block2 = NULL;
                         }
    | IDENT '=' expr ';' {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_ASSIGN;
                            $$->id = $1;
                            $$->expr1 = $3;
                            $$->next = NULL;
                            $$->block1 = NULL;
                            $$->block2 = NULL;
                         }
    | PRINT '(' expr ')' ';' {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_PRINT;
                            $$->expr1 = $3;
                            $$->next = NULL;
                            $$->id = NULL;
                            $$->block1 = NULL;
                            $$->block2 = NULL;
                         }
    | IF '(' expr ')' block ELSE block {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_IF;
                            $$->expr1 = $3;
                            $$->block1 = $5;
                            $$->block2 = $7;
                            $$->next = NULL;
                            $$->id = NULL;
                         }
    | WHILE '(' expr ')' block {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_WHILE;
                            $$->expr1 = $3;
                            $$->block1 = $5;
                            $$->next = NULL;
                            $$->id = NULL;
                            $$->block2 = NULL;
                         }
    | DEF IDENT '(' param_list ')' block {
                            define_func($2, param_list, param_count, $6);
                            param_count = 0;
                            $$ = NULL;
                         }
    | RETURN expr ';' {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_RETURN;
                            $$->expr1 = $2;
                            $$->next = NULL;
                            $$->id = NULL;
                            $$->block1 = NULL;
                            $$->block2 = NULL;
                         }
    ;

block:
      '{' program '}' {
                            $$ = malloc(sizeof(Stmt));
                            $$->type = STMT_BLOCK;
                            $$->block1 = $2;
                            $$->next = NULL;
                            $$->id = NULL;
                            $$->expr1 = NULL;
                            $$->block2 = NULL;
                         }
    ;

param_list:
      param_list ',' IDENT { param_list[param_count++] = $3; $$ = param_list; }
    | IDENT               { param_list[param_count++] = $1; $$ = param_list; }
    |                     { param_count = 0; $$ = NULL; }
    ;

expr:
      NUMBER              { $$ = malloc(sizeof(Expr)); $$->type = EXPR_NUM; $$->data.num = $1; $$->value_type = TYPE_INT; $$->next = NULL; }
    | STRING              { $$ = malloc(sizeof(Expr)); $$->type = EXPR_STR; $$->data.str = $1; $$->value_type = TYPE_STRING; $$->next = NULL; }
    | IDENT               { $$ = malloc(sizeof(Expr)); $$->type = EXPR_VAR; $$->data.id = $1; $$->next = NULL; }
    | expr '+' expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '+'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr '-' expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '-'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr '*' expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '*'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr '/' expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '/'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr EQ expr        { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '='; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr NEQ expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '!'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr '<' expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '<'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr '>' expr       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = '>'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr LE expr        { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = 'L'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | expr GE expr        { $$ = malloc(sizeof(Expr)); $$->type = EXPR_BINOP; $$->data.binop.op = 'G'; $$->data.binop.left = $1; $$->data.binop.right = $3; $$->next = NULL; }
    | '-' expr %prec UMINUS { $$ = malloc(sizeof(Expr)); $$->type = EXPR_NEG; $$->data.subexpr = $2; $$->next = NULL; }
    | IDENT '(' ')'       { $$ = malloc(sizeof(Expr)); $$->type = EXPR_CALL; $$->data.call.id = $1; $$->data.call.args = NULL; $$->data.call.argc = 0; $$->next = NULL; }
    | IDENT '(' expr_list ')' {
                            $$ = malloc(sizeof(Expr));
                            $$->type = EXPR_CALL;
                            $$->data.call.id = $1;
                            int argc = 0;
                            Expr* e = $3;
                            while (e) { argc++; e = e->next; }
                            Expr** args = malloc(argc * sizeof(Expr*));
                            e = $3;
                            for (int i = 0; i < argc; i++) {
                                args[i] = e;
                                e = e->next;
                            }
                            $$->data.call.args = args;
                            $$->data.call.argc = argc;
                            $$->next = NULL;
                         }
    | '(' expr ')'        { $$ = $2; }
    ;

expr_list:
      expr ',' expr_list  { $1->next = $3; $$ = $1; }
    | expr                { $1->next = NULL; $$ = $1; }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    push_scope();
    printf("Enter program source (Ctrl+D to end): \n");
    if (yyparse() == 0) {
        printf("Program parsed successfully.\n Executing...\n");
        execute_stmt(root);
        free_stmt(root);
    }
    pop_scope();
    return 0;
}
