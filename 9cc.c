#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// tokenized results (up to 100 tokens)
Token tokens[100];

// divide string to tokens
void tokenize(char *p) {
    int i = 0;
    while (*p) {

        // skip whitespaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        // addition or subtraction
        if (*p == '+' || *p == '-') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        // integer number
        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        // stop if unexpected string read
        fprintf(stderr, "Failed to tokenize: %s\n", p);
        exit(1);
    }

    // add terminate token
    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

// processing token position
int pos = 0;

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
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// number parser
Node *number() {
    if (tokens[pos].ty == TK_NUM) {
        return new_node_num(tokens[pos++].val);
    }
    error("number expected, but got %s\n", tokens[pos].input);
}

// expression parser
Node *expr() {
    Node *lhs = number();
    for (;;) {
        int op = tokens[pos].ty;
        if (op != '+' && op != '-') {
            break;
        }
        pos++;
        lhs = new_node(op, lhs, number());
    }

    if (tokens[pos].ty != TK_EOF) {
        error("stray token: %s", tokens[pos].input);
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

// IR instructions and position index
IR *ins[1000];
int inp;

// number of register for next use
int regno;

// generate IR instructions
int gen_ir_sub(Node *node) {
    if (node->ty == ND_NUM) {
        int r = regno++;
        ins[inp++] = new_ir(IR_IMM, r, node->val);
        return r;
    }

    assert(node->ty == '+' || node->ty == '-');

    // process child nodes
    int lhs = gen_ir_sub(node->lhs);
    int rhs = gen_ir_sub(node->rhs);

    // execute operator
    ins[inp++] = new_ir(node->ty, lhs, rhs);

    // release register used by rhs
    ins[inp++] = new_ir(IR_KILL, rhs, 0);

    return lhs;
}

int gen_ir(Node *node) {
    int r = gen_ir_sub(node);
    ins[inp++] = new_ir(IR_RETURN, r, 0);
}

// register allocator
char *regs[] = {"rdi", "rsi", "r10", "r11", "r12", "r13", "r14", "r15"};
bool used[8];

//
int reg_map[1000];

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
void alloc_regs() {
    for (int i = 0; i < inp; i++) {
        IR *ir = ins[i];

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
void gen_x86() {
    for (int i = 0; i < inp; i++) {
        IR *ir = ins[i];

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

    // initialize register allocation
    for (int i = 0; i < sizeof(reg_map) / sizeof(*reg_map); i++) {
        reg_map[i] = -1;
    }

    // tokenize and parse
    tokenize(argv[1]);
    Node *node = expr();

    // generate IR and allocate registers
    gen_ir(node);
    alloc_regs();

    // write assembler directives
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // generate assembly code
    gen_x86();
    return 0;
}
