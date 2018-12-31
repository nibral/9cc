#include <ctype.h>
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
    int pos = 0;
    while (*p) {

        // skip whitespaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        // addition or subtraction
        if (*p == '+' || *p == '-') {
            tokens[pos].ty = *p;
            tokens[pos].input = p;
            pos++;
            p++;
            continue;
        }

        // integer number
        if (isdigit(*p)) {
            tokens[pos].ty = TK_NUM;
            tokens[pos].input = p;
            tokens[pos].val = strtol(p, &p, 10);
            pos++;
            continue;
        }

        // stop if unexpected string read
        fprintf(stderr, "Failed to tokenize: %s\n", p);
        exit(1);
    }

    // add terminate token
    tokens[pos].ty = TK_EOF;
    tokens[pos].input = p;
}

// report error
void error(int pos) {
    fprintf(stderr, "Unexpected token: %s\n", tokens[pos].input);
    exit(1);
}

int main(int argc, char **argv) {
    // argument check
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments");
    }

    // tokenize
    tokenize(argv[1]);

    // write assembler directives
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // write first move instruction (expect numbers)
    if (tokens[0].ty != TK_NUM) {
        error(0);
    }
    printf("  mov rax, %d\n", tokens[0].val);

    // write assemblies from tokens like '+ <number>' or '- <number>'
    int pos = 1;
    while (tokens[pos].ty != TK_EOF) {
        // addition
        if (tokens[pos].ty == '+') {
            pos++;
            if (tokens[pos].ty != TK_NUM) {
                error(pos);
            }
            printf("  add rax, %d\n", tokens[pos].val);
            pos++;
            continue;
        }

        // subtraction
        if (tokens[pos].ty == '-') {
            pos++;
            if (tokens[pos].ty != TK_NUM) {
                error(pos);
            }
            printf("  sub rax, %d\n", tokens[pos].val);
            pos++;
            continue;
        }

        // show error if unexpected read
        error(pos);
    }

    printf("  ret\n");
    return 0;
}
