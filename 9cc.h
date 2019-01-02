#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

/// util.c

noreturn void error(char *fmt, ...);

// vector
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vec(void);

void vec_push(Vector *v, void *elem);

/// token.c

// type of token
enum {
    TK_NUM = 256,   // integer
    TK_EOF          // end of input
};

// token
typedef struct {
    int ty;         // type
    int val;        // value of TK_NUM
    char *input;    // token string (for error message)
} Token;

Vector *tokenize(char *p);

/// parse.c

// type of node
enum {
    ND_NUM = 256    // number
};

// node
typedef struct Node {
    int ty;             // type
    struct Node *lhs;   // left hand side
    struct Node *rhs;   // right hand side
    int val;            // value of ND_NUM
} Node;


Node *parse(Vector *tokens);

/// ir.c

// type of IR
enum {
    IR_IMM,
    IR_MOV,
    IR_RETURN,
    IR_KILL,
    IR_NOP,
};

// intermediate representation (IR)
typedef struct {
    int op;
    int lhs;
    int rhs;
} IR;

Vector *gen_ir(Node *node);

/// regalloc.c

extern char *regs[];

void alloc_regs(Vector *irv);

/// codegen.c

void gen_x86(Vector *irv);

