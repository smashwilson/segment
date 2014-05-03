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

static void visit_expr(seg_expr_node *root, seg_ast_visitor visitor, void *state);

static void visit_block(seg_block_node *node, seg_ast_visitor visitor, void *state)
{
  (*(visitor->visit_block_pre))(node, state);

  seg_expr_node *current = node->first;
  while (current != NULL) {
    visit_expr(current, visitor, state);
    current = current->next;
  }

  (*(visitor->visit_block_post))(node, state);
}

/* Visitor walking function. */

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
    visit_block(&(root->child.block), visitor, state);
    break;
  default:
    fprintf(stderr, "Unexpected child_kind in expr: %d\n", root->child_kind);
  }
}

void seg_ast_visit(seg_ast_visitor visitor, seg_block_node *root, void *state)
{
  visit_block(root, visitor, state);
}

void seg_delete_ast_visitor(seg_ast_visitor visitor)
{
  free(visitor);
}
