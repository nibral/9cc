#include "9cc.h"

Map *keywords;

// add token
Token *add_token(Vector *v, int ty, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = ty;
    t->input = input;
    vec_push(v, t);
    return t;
}

// divide string to tokens
static Vector *scan(char *p) {
    Vector *v = new_vec();

    int i = 0;
    while (*p) {

        // skip whitespaces
        if (isspace(*p)) {
            p++;
            continue;
        }

        // single letter token
        if (strchr("+-*/;", *p)) {
            add_token(v, *p, p);
            i++;
            p++;
            continue;
        }

        // keyword
        if (isalpha(*p) || *p == '_') {
            // count length of keyword
            int len = 1;
            while (isalpha(p[len]) || isdigit(p[len]) || p[len] == '_') {
                len++;
            }

            // get value from keyword
            char *name = strndup(p, len);
            int ty = (intptr_t) map_get(keywords, name);
            if (!ty) {
                error("Unknown identifier: %s\n", name);
            }

            add_token(v, ty, p);
            i++;
            p += len;
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
        error("Failed to tokenize: %s\n", p);
    }

    // add terminate token
    add_token(v, TK_EOF, p);

    return v;
}

Vector *tokenize(char *p) {
    keywords = new_map();
    map_put(keywords, "return", (void *) TK_RETURN);

    return scan(p);
}
