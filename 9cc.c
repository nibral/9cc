#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // argument check
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments");
    }
    char *p = argv[1];

    // write assembler directives
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // read first number
    printf("  mov rax, %ld\n", strtol(p, &p,10));

    // read terms
    while (*p) {
        if (*p == '+') {
            p++;
            printf("  add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        // stop if unexpected character read
        fprintf(stderr, "Unexpected character: '%c'\n", *p);
        return 1;
    }

    printf("  ret\n");
    return 0;
}
