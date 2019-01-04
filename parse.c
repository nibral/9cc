#include "9cc.h"

static Vector *tokens;
static int pos;

static void expect(int ty) {
    Token *t = tokens->data[pos];
    if (t->ty != ty) {
        error("%c (%d) expected, but got %c (%d)", ty, ty, t->ty, t->ty);
    }
    pos++;
}

// create new node
static Node *new_node(int op, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = op;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// number parser
static Node *number() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_NUM) {
        error("number expected, but got %s\n", t->input);
    }
    pos++;

    // create number node
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = t->val;
    return node;
}

// multiplication parser
static Node *mul() {
    Node *lhs = number();
    for (;;) {
        Token *t = tokens->data[pos];
        int op = t->ty;
        if (op != '*' && op != '/') {
            return lhs;
        }
        pos++;
        lhs = new_node(op, lhs, number());
    }
}

// expression parser
static Node *expr() {
    Node *lhs = mul();
    for (;;) {
        Token *t = tokens->data[pos];
        int op = t->ty;
        if (op != '+' && op != '-') {
            return lhs;
        }
        pos++;
        lhs = new_node(op, lhs, mul());
    }
}

// generate statements from tokens
static Node *stmt() {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_COMP_STMT;
    node->stmts = new_vec();

    for (;;) {
        Token *t = tokens->data[pos];

        // exit when all tokens processed
        if (t->ty == TK_EOF) {
            return node;
        }

        // add statement
        Node *e = malloc(sizeof(Node));
        if (t->ty == TK_RETURN) {
            pos++;
            e->ty = ND_RETURN;
            e->expr = expr();
        } else {
            e->ty = ND_EXPR_STMT;
            e->expr = expr();
        }
        vec_push(node->stmts, e);

        // confirm that statement ends with a semicolon
        expect(';');
    }
}

Node *parse(Vector *v) {
    tokens = v;
    pos = 0;

    return stmt();
}
