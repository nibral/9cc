#include <ctype.h>
#include <stdarg.h>
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

// code generator
char *regs[] = {"rdi", "rsi", "r10", "r11", "r12", "r13", "r14", "r15", NULL};
int cur;

char *gen(Node *node) {
    if (node->ty == ND_NUM) {
        char *reg = regs[cur++];
        if (!reg) {
            error("register exhausted");
        }
        printf("  mov %s, %d\n", reg, node->val);
        return reg;
    }

    char *dst = gen(node->lhs);
    char *src = gen(node->rhs);

    switch (node->ty) {
        case '+':
            printf("  add %s, %s\n", dst, src);
            return dst;
        case '-':
            printf("  sub %s, %s\n", dst, src);
            return dst;
    }
}

int main(int argc, char **argv) {
    // argument check
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments");
    }

    // tokenize and parse
    tokenize(argv[1]);
    Node *node = expr();

    // write assembler directives
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // generate assembly code
    printf("  mov rax, %s\n", gen(node));
    printf("  ret\n");
    return 0;
}
