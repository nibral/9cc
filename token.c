#include "9cc.h"

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

        // single letter token
        if (strchr("+-*", *p)) {
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
