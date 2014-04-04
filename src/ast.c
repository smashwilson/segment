#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

/* Visitor */

struct seg_ast_visitor {
  seg_integer_handler visit_integer;
  seg_binop_handler visit_binop;
  seg_expr_handler visit_expr;
  seg_statement_handler visit_statement;
  seg_statementlist_handler visit_statementlist;
};

static void visit_integer_null(seg_integer_node *node, void *state) { }
static void visit_binop_null(seg_binop_node *node, void *state) { }
static void visit_expr_null(seg_expr_node *node, void *state) { }
static void visit_statement_null(seg_statement_node *node, void *state) { }
static void visit_statementlist_null(seg_statementlist_node *node, void *state) { }

seg_ast_visitor seg_new_ast_visitor()
{
  seg_ast_visitor visitor = malloc(sizeof(struct seg_ast_visitor));

  visitor->visit_integer = &visit_integer_null;
  visitor->visit_binop = &visit_binop_null;
  visitor->visit_expr = &visit_expr_null;
  visitor->visit_statement = &visit_statement_null;
  visitor->visit_statementlist = &visit_statementlist_null;

  return visitor;
}

void seg_ast_visit_integer(seg_ast_visitor visitor, seg_integer_handler visit)
{
  visitor->visit_integer = visit;
}

void seg_ast_visit_binop(seg_ast_visitor visitor, seg_binop_handler visit)
{
  visitor->visit_binop = visit;
}

void seg_ast_visit_expr(seg_ast_visitor visitor, seg_expr_handler visit)
{
  visitor->visit_expr = visit;
}

void seg_ast_visit_statement(seg_ast_visitor visitor, seg_statement_handler visit)
{
  visitor->visit_statement = visit;
}

void seg_ast_visit_statementlist(seg_ast_visitor visitor, seg_statementlist_handler visit)
{
  visitor->visit_statementlist = visit;
}

/* Forward declarations of visitor walking functions that are used
   recursively. */

static void visit_expr(
  seg_expr_node *root,
  seg_ast_visitor visitor,
  void *state
);

/* Visitor walking functions. */

static void visit_integer(
  seg_integer_node *root,
  seg_ast_visitor visitor,
  void *state
) {
  (*(visitor->visit_integer))(root, state);
}

static void visit_binop(
  seg_binop_node *root,
  seg_ast_visitor visitor,
  void *state
) {
  /* TODO the ordering is wrong here.
     If we're evaluating, we'll want to visit the binop after the
     lhs and rhs. Printing needs this order, though.
     Maybe a visitor flag?
  */
  (*(visitor->visit_binop))(root, state);

  visit_expr(root->left, visitor, state);
  visit_expr(root->right, visitor, state);
}

static void visit_expr(
  seg_expr_node *root,
  seg_ast_visitor visitor,
  void *state
) {
  (*(visitor->visit_expr))(root, state);

  switch(root->kind) {
  case SEG_INTEGER:
      visit_integer(root->expr.integer, visitor, state);
      break;
  case SEG_BINOP:
      visit_binop(root->expr.binop, visitor, state);
      break;
  default:
      fprintf(stderr, "Unexpected kind in expr: %d", root->kind);
  }
}

static void visit_statement(
  seg_statement_node *root,
  seg_ast_visitor visitor,
  void *state
) {
  (*(visitor->visit_statement))(root, state);

  switch(root->child_kind) {
  case SEG_EXPR:
      visit_expr(root->child.expr, visitor, state);
      break;
  default:
      fprintf(stderr, "Unexpected child_kind in statement: %d\n", root->child_kind);
  }
}

static void visit_statementlist(
    seg_statementlist_node *root,
    seg_ast_visitor visitor,
    void *state
) {
  (*(visitor->visit_statementlist))(root, state);

  seg_statement_node *current = root->first;
  while (current != NULL) {
    visit_statement(current, visitor, state);
    current = current->next;
  }
}

void seg_ast_visit(
    seg_ast_visitor visitor,
    seg_statementlist_node *root,
    void *state
) {
  visit_statementlist(root, visitor, state);
}

void seg_delete_ast_visitor(seg_ast_visitor visitor)
{
  free(visitor);
}
