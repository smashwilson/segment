#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "parse_helpers.h"
#include "ast.h"

struct seg_parser_context {
  seg_block_node *block;
  seg_parser_contextp parent;
};

void seg_parser_pushcontext(seg_parser_state *state, seg_block_node *block)
{
  seg_parser_contextp head = malloc(sizeof(struct seg_parser_context));
  head->block = block;
  head->parent = state->context;

  state->context = head;
}

void seg_parser_popcontext(seg_parser_state *state)
{
  seg_parser_contextp head = state->context;
  if (head != NULL) {
    state->context = head->parent;
    free(head);
  }
}

void seg_parser_addparam(seg_parser_state *state, seg_parameter_list *param)
{
  seg_parser_contextp context = state->context;
  if (context == NULL) {
    fputs("Warning: block parameter encountered outside of a block.\n", stderr);
    fputs("How did you even do that?\n", stderr);
    return;
  }

  seg_block_node *block = context->block;
  param->next = block->parameters;
  block->parameters = param;
}

int seg_parser_isarg(seg_parser_state *state, const char *identifier, size_t length)
{
  /*
   Even if you're nesting blocks like a crazy person a simple linear scan should do the trick
   here.
  */
  seg_parser_contextp current = state->context;
  while (current != NULL) {
    /* Match "identifier" to any strings in this block's parameter list. */
    seg_parameter_list *cparam = current->block->parameters;

    while (cparam != NULL) {
      if (cparam->length == length && ! memcmp(identifier, cparam->name, length)) {
        return 1;
      }

      cparam = cparam->next;
    }

    current = current->parent;
  }

  return 0;
}

seg_statementlist_node *seg_append_statement(seg_statementlist_node *list, seg_expr_node *maybe)
{
  if (maybe != NULL) {
    if (list->last != NULL) {
      /* Append maybe to the end of a non-empty list. */
      list->last->next = maybe;
    } else {
      /* This is the first statement. */
      list->first = maybe;
    }

    /* The new non-empty statement is now the last statement in the list. */
    list->last = maybe;
  }

  return list;
}

seg_expr_node *seg_parse_binop(seg_expr_node *lhs, seg_token *op, seg_expr_node *rhs)
{
  seg_methodcall_node *methodcall = malloc(sizeof(seg_methodcall_node));
  methodcall->selector = seg_token_as_string(op, &(methodcall->length));
  seg_delete_token(op);

  methodcall->receiver = lhs;
  methodcall->args = seg_parse_arg(rhs, NULL);

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_METHODCALL;
  out->child.methodcall = methodcall;
  return out;
}

seg_expr_node *seg_parse_methodcall(
  seg_expr_node *receiver,
  seg_token *selector,
  int trim,
  seg_arg_list *args
) {
  /* Extract the selector and destroy its token. */
  seg_methodcall_node *methodcall = malloc(sizeof(seg_methodcall_node));
  if (trim) {
    methodcall->selector = seg_token_without(selector, &(methodcall->length), '(');
  } else {
    methodcall->selector = seg_token_as_string(selector, &(methodcall->length));
  }
  seg_delete_token(selector);

  methodcall->receiver = receiver;
  methodcall->args = args;

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_METHODCALL;
  out->child.methodcall = methodcall;
  return out;
}

seg_arg_list *seg_parse_arg(seg_expr_node *value, seg_token *keyword)
{
  seg_arg_list *arg = malloc(sizeof(seg_arg_list));

  if (keyword != NULL) {
    arg->keyword = seg_token_without(keyword, &(arg->length), ':');
  } else {
    arg->keyword = NULL;
    arg->length = 0;
  }

  arg->value = value;
  arg->next = NULL;

  return arg;
}

seg_expr_node *seg_implicit_self()
{
  seg_var_node *var = malloc(sizeof(seg_var_node));
  var->varname = "self";
  var->length = 4;

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_VAR;
  out->child.var = var;
  return out;
}

seg_arg_list *seg_reverse_args(seg_arg_list *original)
{
  if (original == NULL) {
    return NULL;
  }

  if (original->next == NULL) {
    return original;
  }

  seg_arg_list *rest = original->next;
  seg_arg_list *reversed = seg_reverse_args(rest);

  original->next = NULL;
  rest->next = original;

  return reversed;
}

seg_parameter_list *seg_reverse_params(seg_parameter_list *original)
{
  if (original == NULL) {
    return NULL;
  }

  if (original->next == NULL) {
    return original;
  }

  seg_parameter_list *rest = original->next;
  seg_parameter_list *reversed = seg_reverse_params(rest);

  original->next = NULL;
  rest->next = original;

  return reversed;
}
