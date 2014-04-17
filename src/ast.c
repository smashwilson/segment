#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

/* Visitor */

struct seg_ast_visitor {
  seg_integer_handler visit_integer;
  seg_string_handler visit_string;
  seg_symbol_handler visit_symbol;

  seg_var_handler visit_var;

  seg_methodcall_handler visit_methodcall_pre;
  seg_methodcall_handler visit_methodcall_post;

  seg_block_handler visit_block_pre;
  seg_block_handler visit_block_post;

  seg_statementlist_handler visit_statementlist_pre;
  seg_statementlist_handler visit_statementlist_post;
};

static void visit_null(void *node, void *state) { }

seg_ast_visitor seg_new_ast_visitor()
{
  seg_ast_visitor visitor = malloc(sizeof(struct seg_ast_visitor));

  visitor->visit_integer = (seg_integer_handler) &visit_null;
  visitor->visit_string = (seg_string_handler) &visit_null;
  visitor->visit_symbol = (seg_symbol_handler) &visit_null;

  visitor->visit_var = (seg_var_handler) &visit_null;

  visitor->visit_methodcall_pre = (seg_methodcall_handler) &visit_null;
  visitor->visit_methodcall_post = (seg_methodcall_handler) &visit_null;

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

void seg_ast_visit_symbol(seg_ast_visitor visitor, seg_symbol_handler visit)
{
  visitor->visit_symbol = visit;
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

static void visit_block(seg_block_node *root, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_block_pre))(root, state);
  visit_statementlist(root->body, visitor, state);
  (*(visitor->visit_block_post))(root, state);
}

static void visit_expr(seg_expr_node *root, seg_ast_visitor visitor, void *state)
{
  switch(root->child_kind) {
  case SEG_INTEGER:
    (*(visitor->visit_integer))(&(root->child.integer), state);
    break;
  case SEG_STRING:
    (*(visitor->visit_string))(&(root->child.string), state);
    break;
  case SEG_SYMBOL:
    (*(visitor->visit_symbol))(&(root->child.symbol), state);
    break;
  case SEG_VAR:
    (*(visitor->visit_var))(&(root->child.var), state);
    break;
  case SEG_METHODCALL:
    (*(visitor->visit_methodcall_pre))(&(root->child.methodcall), state);

    visit_expr(root->child.methodcall.receiver, visitor, state);

    seg_arg_list *current = root->child.methodcall.args;
    while (current != NULL) {
      visit_expr(current->value, visitor, state);
      current = current->next;
    }

    (*(visitor->visit_methodcall_post))(&(root->child.methodcall), state);
    break;
  case SEG_BLOCK:
    (*(visitor->visit_block_pre))(&(root->child.block), state);

    visit_statementlist(root->child.block.body, visitor, state);

    (*(visitor->visit_block_post))(&(root->child.block), state);
    break;
  default:
    fprintf(stderr, "Unexpected child_kind in expr: %d\n", root->child_kind);
  }
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
