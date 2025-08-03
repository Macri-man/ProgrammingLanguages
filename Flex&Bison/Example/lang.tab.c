/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "lang.y"

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


#line 406 "lang.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lang.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NUMBER = 3,                     /* NUMBER  */
  YYSYMBOL_IDENT = 4,                      /* IDENT  */
  YYSYMBOL_STRING = 5,                     /* STRING  */
  YYSYMBOL_IF = 6,                         /* IF  */
  YYSYMBOL_ELSE = 7,                       /* ELSE  */
  YYSYMBOL_WHILE = 8,                      /* WHILE  */
  YYSYMBOL_PRINT = 9,                      /* PRINT  */
  YYSYMBOL_DEF = 10,                       /* DEF  */
  YYSYMBOL_RETURN = 11,                    /* RETURN  */
  YYSYMBOL_EQ = 12,                        /* EQ  */
  YYSYMBOL_NEQ = 13,                       /* NEQ  */
  YYSYMBOL_LE = 14,                        /* LE  */
  YYSYMBOL_GE = 15,                        /* GE  */
  YYSYMBOL_16_ = 16,                       /* '<'  */
  YYSYMBOL_17_ = 17,                       /* '>'  */
  YYSYMBOL_18_ = 18,                       /* '+'  */
  YYSYMBOL_19_ = 19,                       /* '-'  */
  YYSYMBOL_20_ = 20,                       /* '*'  */
  YYSYMBOL_21_ = 21,                       /* '/'  */
  YYSYMBOL_UMINUS = 22,                    /* UMINUS  */
  YYSYMBOL_23_ = 23,                       /* ';'  */
  YYSYMBOL_24_ = 24,                       /* '='  */
  YYSYMBOL_25_ = 25,                       /* '('  */
  YYSYMBOL_26_ = 26,                       /* ')'  */
  YYSYMBOL_27_ = 27,                       /* '{'  */
  YYSYMBOL_28_ = 28,                       /* '}'  */
  YYSYMBOL_29_ = 29,                       /* ','  */
  YYSYMBOL_YYACCEPT = 30,                  /* $accept  */
  YYSYMBOL_program = 31,                   /* program  */
  YYSYMBOL_statement = 32,                 /* statement  */
  YYSYMBOL_block = 33,                     /* block  */
  YYSYMBOL_param_list = 34,                /* param_list  */
  YYSYMBOL_expr = 35,                      /* expr  */
  YYSYMBOL_expr_list = 36                  /* expr_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   198

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  33
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  77

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   271


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      25,    26,    20,    18,    29,    19,     2,    21,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    23,
      16,    24,    17,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    27,     2,    28,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    22
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   366,   366,   376,   380,   389,   398,   407,   416,   425,
     430,   442,   454,   455,   456,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   492,   496,   497
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NUMBER", "IDENT",
  "STRING", "IF", "ELSE", "WHILE", "PRINT", "DEF", "RETURN", "EQ", "NEQ",
  "LE", "GE", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "UMINUS", "';'",
  "'='", "'('", "')'", "'{'", "'}'", "','", "$accept", "program",
  "statement", "block", "param_list", "expr", "expr_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-24)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -24,    25,   -24,   -24,   -21,   -24,     2,    13,    18,    44,
       7,     7,     7,   -24,   143,     7,    36,     7,     7,     7,
      35,    43,   155,   -24,    83,     7,     7,     7,     7,     7,
       7,     7,     7,     7,     7,   -24,   167,   -24,    65,    46,
      98,   113,   128,    66,   -24,   -24,   177,   177,    45,    45,
      45,    45,    -7,    -7,   -24,   -24,   -24,     7,   -24,    42,
      42,    51,   -24,    16,   -24,   -24,    64,   -24,   -24,    42,
      71,    48,    42,   -24,   -24,   -24,   -24
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     1,    15,    17,    16,     0,     0,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,     0,
       0,    17,     0,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,     0,    29,    33,     0,
       0,     0,     0,    14,    10,    31,    22,    23,    26,    27,
      24,    25,    18,    19,    20,    21,     5,     0,    30,     0,
       0,     0,    13,     0,    32,     3,     0,     8,     6,     0,
       0,     0,     0,     9,    12,    11,     7
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -24,    22,   -24,   -23,   -24,   -10,    31
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,    13,    66,    63,    14,    39
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      22,    23,    24,    15,    16,    36,    38,    40,    41,    42,
       3,    21,     5,    33,    34,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,     2,    11,    17,     3,     4,
       5,     6,    12,     7,     8,     9,    10,    67,    18,     3,
      21,     5,    69,    19,    11,    70,    73,    38,    20,    76,
      12,     3,     4,     5,     6,    11,     7,     8,     9,    10,
      43,    12,    37,    31,    32,    33,    34,    11,    16,    65,
      62,    72,    58,    12,    68,    74,    75,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    71,    64,     0,
       0,     0,     0,     0,    57,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,     0,     0,     0,    45,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       0,     0,     0,     0,    59,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,     0,     0,     0,    60,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
       0,     0,     0,     0,    61,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,    35,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,     0,    44,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,     0,
      56,    27,    28,    29,    30,    31,    32,    33,    34
};

static const yytype_int8 yycheck[] =
{
      10,    11,    12,    24,    25,    15,    16,    17,    18,    19,
       3,     4,     5,    20,    21,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,    19,    25,     3,     4,
       5,     6,    25,     8,     9,    10,    11,    60,    25,     3,
       4,     5,    26,    25,    19,    29,    69,    57,     4,    72,
      25,     3,     4,     5,     6,    19,     8,     9,    10,    11,
      25,    25,    26,    18,    19,    20,    21,    19,    25,    27,
       4,     7,    26,    25,    23,     4,    28,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    65,    57,    -1,
      -1,    -1,    -1,    -1,    29,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,    26,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    26,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,    26,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    26,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    -1,    23,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    23,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    -1,
      23,    14,    15,    16,    17,    18,    19,    20,    21
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    31,     0,     3,     4,     5,     6,     8,     9,    10,
      11,    19,    25,    32,    35,    24,    25,    25,    25,    25,
       4,     4,    35,    35,    35,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    23,    35,    26,    35,    36,
      35,    35,    35,    25,    23,    26,    35,    35,    35,    35,
      35,    35,    35,    35,    35,    35,    23,    29,    26,    26,
      26,    26,     4,    34,    36,    27,    33,    33,    23,    26,
      29,    31,     7,    33,     4,    28,    33
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    30,    31,    31,    32,    32,    32,    32,    32,    32,
      32,    33,    34,    34,    34,    35,    35,    35,    35,    35,
      35,    35,    35,    35,    35,    35,    35,    35,    35,    35,
      35,    35,    36,    36
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     0,     2,     4,     5,     7,     5,     6,
       3,     3,     3,     1,     0,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       4,     3,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: program statement  */
#line 366 "lang.y"
                         { if((yyvsp[0].stmt)) { 
                              if (!(yyvsp[-1].stmt)) (yyval.stmt) = (yyvsp[0].stmt); 
                              else { 
                                Stmt* s = (yyvsp[-1].stmt); 
                                while(s->next) s=s->next; 
                                s->next = (yyvsp[0].stmt); 
                                (yyval.stmt) = (yyvsp[-1].stmt); 
                              } 
                            root = (yyval.stmt); }
                         }
#line 1497 "lang.tab.c"
    break;

  case 3: /* program: %empty  */
#line 376 "lang.y"
                        { (yyval.stmt) = NULL; root = NULL; }
#line 1503 "lang.tab.c"
    break;

  case 4: /* statement: expr ';'  */
#line 380 "lang.y"
                         {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_EXPR;
                            (yyval.stmt)->expr1 = (yyvsp[-1].expr);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->id = NULL;
                            (yyval.stmt)->block1 = NULL;
                            (yyval.stmt)->block2 = NULL;
                         }
#line 1517 "lang.tab.c"
    break;

  case 5: /* statement: IDENT '=' expr ';'  */
#line 389 "lang.y"
                         {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_ASSIGN;
                            (yyval.stmt)->id = (yyvsp[-3].str);
                            (yyval.stmt)->expr1 = (yyvsp[-1].expr);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->block1 = NULL;
                            (yyval.stmt)->block2 = NULL;
                         }
#line 1531 "lang.tab.c"
    break;

  case 6: /* statement: PRINT '(' expr ')' ';'  */
#line 398 "lang.y"
                             {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_PRINT;
                            (yyval.stmt)->expr1 = (yyvsp[-2].expr);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->id = NULL;
                            (yyval.stmt)->block1 = NULL;
                            (yyval.stmt)->block2 = NULL;
                         }
#line 1545 "lang.tab.c"
    break;

  case 7: /* statement: IF '(' expr ')' block ELSE block  */
#line 407 "lang.y"
                                       {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_IF;
                            (yyval.stmt)->expr1 = (yyvsp[-4].expr);
                            (yyval.stmt)->block1 = (yyvsp[-2].stmt);
                            (yyval.stmt)->block2 = (yyvsp[0].stmt);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->id = NULL;
                         }
#line 1559 "lang.tab.c"
    break;

  case 8: /* statement: WHILE '(' expr ')' block  */
#line 416 "lang.y"
                               {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_WHILE;
                            (yyval.stmt)->expr1 = (yyvsp[-2].expr);
                            (yyval.stmt)->block1 = (yyvsp[0].stmt);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->id = NULL;
                            (yyval.stmt)->block2 = NULL;
                         }
#line 1573 "lang.tab.c"
    break;

  case 9: /* statement: DEF IDENT '(' param_list ')' block  */
#line 425 "lang.y"
                                         {
                            define_func((yyvsp[-4].str), param_list, param_count, (yyvsp[0].stmt));
                            param_count = 0;
                            (yyval.stmt) = NULL;
                         }
#line 1583 "lang.tab.c"
    break;

  case 10: /* statement: RETURN expr ';'  */
#line 430 "lang.y"
                      {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_RETURN;
                            (yyval.stmt)->expr1 = (yyvsp[-1].expr);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->id = NULL;
                            (yyval.stmt)->block1 = NULL;
                            (yyval.stmt)->block2 = NULL;
                         }
#line 1597 "lang.tab.c"
    break;

  case 11: /* block: '{' program '}'  */
#line 442 "lang.y"
                      {
                            (yyval.stmt) = malloc(sizeof(Stmt));
                            (yyval.stmt)->type = STMT_BLOCK;
                            (yyval.stmt)->block1 = (yyvsp[-1].stmt);
                            (yyval.stmt)->next = NULL;
                            (yyval.stmt)->id = NULL;
                            (yyval.stmt)->expr1 = NULL;
                            (yyval.stmt)->block2 = NULL;
                         }
#line 1611 "lang.tab.c"
    break;

  case 12: /* param_list: param_list ',' IDENT  */
#line 454 "lang.y"
                           { param_list[param_count++] = (yyvsp[0].str); (yyval.strlist) = param_list; }
#line 1617 "lang.tab.c"
    break;

  case 13: /* param_list: IDENT  */
#line 455 "lang.y"
                          { param_list[param_count++] = (yyvsp[0].str); (yyval.strlist) = param_list; }
#line 1623 "lang.tab.c"
    break;

  case 14: /* param_list: %empty  */
#line 456 "lang.y"
                          { param_count = 0; (yyval.strlist) = NULL; }
#line 1629 "lang.tab.c"
    break;

  case 15: /* expr: NUMBER  */
#line 460 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_NUM; (yyval.expr)->data.num = (yyvsp[0].num); (yyval.expr)->value_type = TYPE_INT; (yyval.expr)->next = NULL; }
#line 1635 "lang.tab.c"
    break;

  case 16: /* expr: STRING  */
#line 461 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_STR; (yyval.expr)->data.str = (yyvsp[0].str); (yyval.expr)->value_type = TYPE_STRING; (yyval.expr)->next = NULL; }
#line 1641 "lang.tab.c"
    break;

  case 17: /* expr: IDENT  */
#line 462 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_VAR; (yyval.expr)->data.id = (yyvsp[0].str); (yyval.expr)->next = NULL; }
#line 1647 "lang.tab.c"
    break;

  case 18: /* expr: expr '+' expr  */
#line 463 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '+'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1653 "lang.tab.c"
    break;

  case 19: /* expr: expr '-' expr  */
#line 464 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '-'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1659 "lang.tab.c"
    break;

  case 20: /* expr: expr '*' expr  */
#line 465 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '*'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1665 "lang.tab.c"
    break;

  case 21: /* expr: expr '/' expr  */
#line 466 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '/'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1671 "lang.tab.c"
    break;

  case 22: /* expr: expr EQ expr  */
#line 467 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '='; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1677 "lang.tab.c"
    break;

  case 23: /* expr: expr NEQ expr  */
#line 468 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '!'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1683 "lang.tab.c"
    break;

  case 24: /* expr: expr '<' expr  */
#line 469 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '<'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1689 "lang.tab.c"
    break;

  case 25: /* expr: expr '>' expr  */
#line 470 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = '>'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1695 "lang.tab.c"
    break;

  case 26: /* expr: expr LE expr  */
#line 471 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = 'L'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1701 "lang.tab.c"
    break;

  case 27: /* expr: expr GE expr  */
#line 472 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_BINOP; (yyval.expr)->data.binop.op = 'G'; (yyval.expr)->data.binop.left = (yyvsp[-2].expr); (yyval.expr)->data.binop.right = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1707 "lang.tab.c"
    break;

  case 28: /* expr: '-' expr  */
#line 473 "lang.y"
                            { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_NEG; (yyval.expr)->data.subexpr = (yyvsp[0].expr); (yyval.expr)->next = NULL; }
#line 1713 "lang.tab.c"
    break;

  case 29: /* expr: IDENT '(' ')'  */
#line 474 "lang.y"
                          { (yyval.expr) = malloc(sizeof(Expr)); (yyval.expr)->type = EXPR_CALL; (yyval.expr)->data.call.id = (yyvsp[-2].str); (yyval.expr)->data.call.args = NULL; (yyval.expr)->data.call.argc = 0; (yyval.expr)->next = NULL; }
#line 1719 "lang.tab.c"
    break;

  case 30: /* expr: IDENT '(' expr_list ')'  */
#line 475 "lang.y"
                              {
                            (yyval.expr) = malloc(sizeof(Expr));
                            (yyval.expr)->type = EXPR_CALL;
                            (yyval.expr)->data.call.id = (yyvsp[-3].str);
                            int argc = 0;
                            Expr* e = (yyvsp[-1].expr);
                            while (e) { argc++; e = e->next; }
                            Expr** args = malloc(argc * sizeof(Expr*));
                            e = (yyvsp[-1].expr);
                            for (int i = 0; i < argc; i++) {
                                args[i] = e;
                                e = e->next;
                            }
                            (yyval.expr)->data.call.args = args;
                            (yyval.expr)->data.call.argc = argc;
                            (yyval.expr)->next = NULL;
                         }
#line 1741 "lang.tab.c"
    break;

  case 31: /* expr: '(' expr ')'  */
#line 492 "lang.y"
                          { (yyval.expr) = (yyvsp[-1].expr); }
#line 1747 "lang.tab.c"
    break;

  case 32: /* expr_list: expr ',' expr_list  */
#line 496 "lang.y"
                          { (yyvsp[-2].expr)->next = (yyvsp[0].expr); (yyval.expr) = (yyvsp[-2].expr); }
#line 1753 "lang.tab.c"
    break;

  case 33: /* expr_list: expr  */
#line 497 "lang.y"
                          { (yyvsp[0].expr)->next = NULL; (yyval.expr) = (yyvsp[0].expr); }
#line 1759 "lang.tab.c"
    break;


#line 1763 "lang.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 500 "lang.y"


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
