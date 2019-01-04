#include "9cc.h"

static Vector *code;

// add IR
static IR *add(int op, int lhs, int rhs) {
    IR *ir = malloc(sizeof(IR));
    ir->op = op;
    ir->lhs = lhs;
    ir->rhs = rhs;
    vec_push(code, ir);
    return ir;
}

// generate IR expression
static int gen_expr(Node *node) {
    static int regno;

    if (node->ty == ND_NUM) {
        int r = regno++;
        add(IR_IMM, r, node->val);
        return r;
    }

    assert(strchr("+-*/", node->ty));

    // process child nodes
    int lhs = gen_expr(node->lhs);
    int rhs = gen_expr( node->rhs);

    // execute operator
    add(node->ty, lhs, rhs);

    // release register used by rhs
    add(IR_KILL, rhs, 0);

    return lhs;
}

// generate IR statement
static void gen_stmt(Node *node) {
    if (node->ty == ND_RETURN) {
        int r = gen_expr(node->expr);
        add(IR_RETURN, r, 0);
        add(IR_KILL, r, 0);
        return;
    }

    if (node->ty == ND_EXPR_STMT) {
        int r = gen_expr(node->expr);
        add(IR_KILL, r, 0);
        return;
    }

    if (node->ty == ND_COMP_STMT) {
        for (int i = 0; i < node->stmts->len; i++) {
            gen_stmt(node->stmts->data[i]);
        }
        return;
    }

    error("Unknown node: %d\n", node->ty);
}

Vector *gen_ir(Node *node) {
    assert(node->ty == ND_COMP_STMT);
    code = new_vec();
    gen_stmt(node);
    return code;
}
