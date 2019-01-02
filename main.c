#include "9cc.h"

int main(int argc, char **argv) {
    // argument check
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments");
    }

    // tokenize and parse
    Vector *tokens = tokenize(argv[1]);
    Node *node = parse(tokens);

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
