#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

/* Visitor */

struct seg_ast_visitor {
  seg_integer_handler visit_integer;

  seg_string_handler visit_string;

  seg_var_handler visit_var;

  seg_methodcall_handler visit_methodcall_pre;
  seg_methodcall_handler visit_methodcall_post;

  seg_block_handler visit_block_pre;
  seg_block_handler visit_block_post;

  seg_expr_handler visit_expr_pre;
  seg_expr_handler visit_expr_post;

  seg_statementlist_handler visit_statementlist_pre;
  seg_statementlist_handler visit_statementlist_post;
};

static void visit_null(void *node, void *state) { }

seg_ast_visitor seg_new_ast_visitor()
{
  seg_ast_visitor visitor = malloc(sizeof(struct seg_ast_visitor));

  visitor->visit_integer = (seg_integer_handler) &visit_null;

  visitor->visit_string = (seg_string_handler) &visit_null;

  visitor->visit_var = (seg_var_handler) &visit_null;

  visitor->visit_methodcall_pre = (seg_methodcall_handler) &visit_null;
  visitor->visit_methodcall_post = (seg_methodcall_handler) &visit_null;

  visitor->visit_expr_pre = (seg_expr_handler) &visit_null;
  visitor->visit_expr_post = (seg_expr_handler) &visit_null;

  visitor->visit_block_pre = (seg_block_handler) &visit_null;
  visitor->visit_block_post = (seg_block_handler) &visit_null;

  visitor->visit_statementlist_pre = (seg_statementlist_handler) &visit_null;
  visitor->visit_statementlist_post = (seg_statementlist_handler) &visit_null;

  return visitor;
}

void seg_ast_visit_integer(seg_ast_visitor visitor, seg_integer_handler visit)
{
  visitor->visit_integer = visit;
}

void seg_ast_visit_string(seg_ast_visitor visitor, seg_string_handler visit)
{
  visitor->visit_string = visit;
}

void seg_ast_visit_methodcall(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_methodcall_handler visit
) {
  if (when == SEG_VISIT_PRE) {
    visitor->visit_methodcall_pre = visit;
  } else {
    visitor->visit_methodcall_post = visit;
  }
}

void seg_ast_visit_var(seg_ast_visitor visitor, seg_var_handler visit)
{
  visitor->visit_var = visit;
}

void seg_ast_visit_block(seg_ast_visitor visitor, seg_visit_when when, seg_block_handler visit)
{
  if (when == SEG_VISIT_PRE) {
    visitor->visit_block_pre = visit;
  } else {
    visitor->visit_block_post = visit;
  }
}

void seg_ast_visit_expr(seg_ast_visitor visitor, seg_visit_when when, seg_expr_handler visit)
{
  if (when == SEG_VISIT_PRE) {
    visitor->visit_expr_pre = visit;
  } else {
    visitor->visit_expr_post = visit;
  }
}

void seg_ast_visit_statementlist(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_statementlist_handler visit
) {
  if (when == SEG_VISIT_PRE) {
    visitor->visit_statementlist_pre = visit;
  } else {
    visitor->visit_statementlist_post = visit;
  }
}

/* Forward declarations of visitor walking functions that are used
   recursively. */

static void visit_expr(seg_expr_node *root, seg_ast_visitor visitor, void *state);
static void visit_statementlist(seg_statementlist_node *root, seg_ast_visitor visitor, void *state);

/* Visitor walking functions. */

static void visit_integer(seg_integer_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_integer))(root, state);
}

static void visit_string(seg_string_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_string))(root, state);
}

static void visit_var(seg_var_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_var))(root, state);
}

static void visit_methodcall(seg_methodcall_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_methodcall_pre))(root, state);

  visit_expr(root->receiver, visitor, state);

  seg_arg_list *current = root->args;
  while (current != NULL) {
    visit_expr(current->value, visitor, state);
    current = current->next;
  }

  (*(visitor->visit_methodcall_post))(root, state);
}

static void visit_block(seg_block_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_block_pre))(root, state);
  visit_statementlist(root->body, visitor, state);
  (*(visitor->visit_block_post))(root, state);
}

static void visit_expr(seg_expr_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_expr_pre))(root, state);

  switch(root->child_kind) {
  case SEG_INTEGER:
    visit_integer(root->child.integer, visitor, state);
    break;
  case SEG_STRING:
    visit_string(root->child.string, visitor, state);
  case SEG_VAR:
    visit_var(root->child.var, visitor, state);
    break;
  case SEG_METHODCALL:
    visit_methodcall(root->child.methodcall, visitor, state);
    break;
  case SEG_BLOCK:
    visit_block(root->child.block, visitor, state);
    break;
  default:
    fprintf(stderr, "Unexpected child_kind in expr: %d\n", root->child_kind);
  }

  (*(visitor->visit_expr_post))(root, state);
}

static void visit_statementlist(seg_statementlist_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_statementlist_pre))(root, state);

  seg_expr_node *current = root->first;
  while (current != NULL) {
    visit_expr(current, visitor, state);
    current = current->next;
  }

  (*(visitor->visit_statementlist_post))(root, state);
}

void seg_ast_visit(seg_ast_visitor visitor, seg_statementlist_node *root, void *state)
{
  visit_statementlist(root, visitor, state);
}

void seg_delete_ast_visitor(seg_ast_visitor visitor)
{
  free(visitor);
}
