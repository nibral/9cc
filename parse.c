#include "9cc.h"

static Vector *tokens;
static int pos;

// create new node
static Node *new_node(int op, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = op;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// create new number node
static Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

// number parser
static Node *number() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_NUM) {
        error("number expected, but got %s\n", t->input);
    }
    pos++;
    return new_node_num(t->val);
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

Node *parse(Vector *v) {
    tokens = v;
    pos = 0;
    Node *node = expr();

    Token *t = tokens->data[pos];
    if (t->ty != TK_EOF) {
        error("stray token: %s", t->input);
    }
    return node;
}
