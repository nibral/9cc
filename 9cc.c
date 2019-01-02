#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

// vector
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

// create new vector
Vector *new_vec() {
    Vector *v = malloc(sizeof(Vector));
    v->data = malloc(sizeof(void *) * 16);
    v->capacity = 16;
    v->len = 0;
    return v;
}

// push data to vector
void vec_push(Vector *v, void *elem) {
    // expand if capacity exhaust
    if (v->len == v->capacity) {
        v->capacity *= 2;
        v->data = realloc(v->data, sizeof(void *) * v->capacity);
    }
    v->data[v->len++] = elem;
}

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

// add token
Token *add_token(Vector *v, int ty, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = ty;
    t->input = input;
    vec_push(v, t);
    return t;
}

// divide string to tokens
Vector *tokenize(char *p) {
    Vector *v = new_vec();

    int i = 0;
    while (*p) {

        // skip whitespaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        // addition or subtraction
        if (*p == '+' || *p == '-') {
            add_token(v, *p, p);
            i++;
            p++;
            continue;
        }

        // integer number
        if (isdigit(*p)) {
            Token *t = add_token(v, TK_NUM, p);
            t->val = strtol(p, &p, 10);
            i++;
            continue;
        }

        // stop if unexpected string read
        fprintf(stderr, "Failed to tokenize: %s\n", p);
        exit(1);
    }

    // add terminate token
    add_token(v, TK_EOF, p);

    return v;
}

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

Vector *tokens;
int pos;

// create new node
Node *new_node(int op, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = op;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// create new number node
Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

// report error
noreturn void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// number parser
Node *number() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_NUM) {
        error("number expected, but got %s\n", t->input);
    }
    pos++;
    return new_node_num(t->val);
}

// expression parser
Node *expr() {
    Node *lhs = number();
    for (;;) {
        Token *t = tokens->data[pos];
        int op = t->ty;
        if (op != '+' && op != '-') {
            break;
        }
        pos++;
        lhs = new_node(op, lhs, number());
    }

    Token *t = tokens->data[pos];
    if (t->ty != TK_EOF) {
        error("stray token: %s", t->input);
    }
    return lhs;
}

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

// create new IR
IR *new_ir(int op, int lhs, int rhs) {
    IR *ir = malloc(sizeof(IR));
    ir->op = op;
    ir->lhs = lhs;
    ir->rhs = rhs;
    return ir;
}

// generate IR instructions
int gen_ir_sub(Vector *v, Node *node) {
    static int regno;

    if (node->ty == ND_NUM) {
        int r = regno++;
        vec_push(v, new_ir(IR_IMM, r, node->val));
        return r;
    }

    assert(node->ty == '+' || node->ty == '-');

    // process child nodes
    int lhs = gen_ir_sub(v, node->lhs);
    int rhs = gen_ir_sub(v, node->rhs);

    // execute operator
    vec_push(v, new_ir(node->ty, lhs, rhs));

    // release register used by rhs
    vec_push(v, new_ir(IR_KILL, rhs, 0));

    return lhs;
}

Vector *gen_ir(Node *node) {
    Vector *v = new_vec();
    int r = gen_ir_sub(v, node);
    vec_push(v, new_ir(IR_RETURN, r, 0));
    return v;
}

// register allocator
char *regs[] = {"rdi", "rsi", "r10", "r11", "r12", "r13", "r14", "r15"};
bool used[sizeof(regs) / sizeof(*regs)];
int *reg_map;

// find index of register to use
int alloc(int ir_reg) {
    if (reg_map[ir_reg] != -1) {
        int r = reg_map[ir_reg];
        assert(used[r]);
        return r;
    }

    // use first of unallocated register
    for (int i = 0; i < sizeof(regs) / sizeof(*regs); i++) {
        if (used[i]) {
            continue;
        }
        used[i] = true;
        reg_map[ir_reg] = i;
        return i;
    }

    error("register exhausted");
}

// release register allocation
void kill(int r) {
    assert(used[r]);
    used[r] = false;
}

// allocate registers for all IR
void alloc_regs(Vector *irv) {
    // initialize IR vector
    reg_map = malloc(sizeof(int) * irv->len);
    for (int i = 0; i < irv->len; i++) {
        reg_map[i] = -1;
    }

    for (int i = 0; i < irv->len; i++) {
        IR *ir = irv->data[i];

        switch (ir->op) {
            case IR_IMM:
                ir->lhs = alloc(ir->lhs);
                break;
            case IR_MOV:
            case '+':
            case '-':
                ir->lhs = alloc(ir->lhs);
                ir->rhs = alloc(ir->rhs);
                break;
            case IR_RETURN:
                kill(reg_map[ir->lhs]);
                break;
            case IR_KILL:
                kill(reg_map[ir->lhs]);
                ir->op = IR_NOP;
                break;
            default:
                assert(0 && "unknown operator");
        }
    }
}

// code generator
void gen_x86(Vector *irv) {
    for (int i = 0; i < irv->len; i++) {
        IR *ir = irv->data[i];

        switch (ir->op) {
            case IR_IMM:
                printf("  mov %s, %d\n", regs[ir->lhs], ir->rhs);
                break;
            case IR_MOV:
                printf("  mov %s, %s\n", regs[ir->lhs], regs[ir->rhs]);
                break;
            case IR_RETURN:
                printf("  mov rax, %s\n", regs[ir->lhs]);
                printf("  ret\n");
                break;
            case '+':
                printf("  add %s, %s\n", regs[ir->lhs], regs[ir->rhs]);
                break;
            case '-':
                printf("  sub %s, %s\n", regs[ir->lhs], regs[ir->rhs]);
                break;
            case IR_NOP:
                break;
            default:
                assert(0 && "unknown operator");
        }
    }
}

int main(int argc, char **argv) {
    // argument check
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments");
    }

    // tokenize and parse
    tokens = tokenize(argv[1]);
    Node *node = expr();

    // generate IR and allocate registers
    Vector *irv = gen_ir(node);
    alloc_regs(irv);

    // write assembler directives
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // generate assembly code
    gen_x86(irv);
    return 0;
}
