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

// report error
void error(int i) {
    fprintf(stderr, "Unexpected token: %s\n", tokens[i].input);
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
    int i = 1;
    while (tokens[i].ty != TK_EOF) {
        // addition
        if (tokens[i].ty == '+') {
            i++;
            if (tokens[i].ty != TK_NUM) {
                error(i);
            }
            printf("  add rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        // subtraction
        if (tokens[i].ty == '-') {
            i++;
            if (tokens[i].ty != TK_NUM) {
                error(i);
            }
            printf("  sub rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        // show error if unexpected read
        error(i);
    }

    printf("  ret\n");
    return 0;
}
