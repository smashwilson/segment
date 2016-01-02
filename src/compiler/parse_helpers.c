#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "compiler/parse_helpers.h"
#include "compiler/ast.h"

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
  seg_object existing = seg_symboltable_get(state->symboltable, identifier, length);
  if (SEG_SAME(existing, SEG_NO_SYMBOL)) {
    /* If it wasn't intern'd, it's not an argument. */
    // FIXME this isn't true if the symbol is an immediate.
    return 0;
  }

  seg_parser_contextp current = state->context;
  while (current != NULL) {
    /* Match "identifier" to any strings in this block's parameter list. */
    seg_parameter_list *cparam = current->block->parameters;

    while (cparam != NULL) {
      if (SEG_SAME(cparam->parameter, existing)) {
        return 1;
      }

      cparam = cparam->next;
    }

    current = current->parent;
  }

  return 0;
}

seg_block_node *seg_append_expr(seg_block_node *list, seg_expr_node *maybe)
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

seg_expr_node *seg_parse_binop(
  seg_parser_state *state,
  seg_expr_node *lhs,
  seg_token *op,
  seg_expr_node *rhs
) {
  char *selname;
  size_t length;

  selname = seg_token_as_string(op, &length);
  seg_delete_token(op);

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_METHODCALL;
  seg_symboltable_intern(state->symboltable, selname, length, &out->child.methodcall.selector);
  out->child.methodcall.receiver = lhs;
  out->child.methodcall.args = seg_parse_arg(state, rhs, NULL);
  return out;
}

seg_expr_node *seg_parse_methodcall(
  seg_parser_state *state,
  seg_expr_node *receiver,
  seg_token *selector,
  int trim,
  seg_arg_list *args
) {
  char *selname;
  size_t length;

  /* Extract the selector and destroy its token. */
  if (trim) {
    selname = seg_token_without(selector, 0, 1, &length);
  } else {
    selname = seg_token_as_string(selector, &length);
  }
  seg_delete_token(selector);

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_METHODCALL;
  seg_symboltable_intern(state->symboltable, selname, length, &out->child.methodcall.selector);
  out->child.methodcall.receiver = receiver;
  out->child.methodcall.args = args;
  return out;
}

seg_expr_node *seg_implicit_methodcall(
  seg_parser_state *state,
  seg_expr_node *receiver,
  const char *selector
) {
  seg_object selsym;
  seg_symboltable_intern(state->symboltable, selector, strlen(selector), &selsym);

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_METHODCALL;
  out->child.methodcall.selector = selsym;
  out->child.methodcall.receiver = receiver;
  out->child.methodcall.args = NULL;
  return out;
}

seg_expr_node *seg_parse_interpolation(
  seg_parser_state *state,
  seg_expr_node *stem,
  seg_expr_node *middle,
  seg_token *end_token
) {
  middle->child.methodcall.receiver = stem;

  /* } ... " */
  size_t end_length;
  char *end_content = seg_token_without(end_token, 1, 1, &end_length);
  seg_delete_token(end_token);

  if (end_content != NULL) {
    seg_expr_node *end_node = malloc(sizeof(seg_expr_node));
    end_node->child_kind = SEG_STRING;
    end_node->child.string.value = end_content;
    end_node->child.string.length = end_length;

    seg_arg_list *args = seg_parse_arg(state, end_node, NULL);
    args->next = middle->child.methodcall.args;
    middle->child.methodcall.args = args;
  }

  /* Arguments are reversed. */
  middle->child.methodcall.args = seg_reverse_args(middle->child.methodcall.args);

  return middle;
}

seg_arg_list *seg_parse_arg(seg_parser_state *state, seg_expr_node *value, seg_token *keyword)
{
  seg_arg_list *arg = malloc(sizeof(seg_arg_list));

  if (keyword != NULL) {
    char *kwname;
    size_t length;

    kwname = seg_token_without(keyword, 0, 1, &length);

    seg_symboltable_intern(state->symboltable, kwname, length, &arg->keyword);
  } else {
    arg->keyword.pointer = NULL;
  }

  arg->value = value;
  arg->next = NULL;

  return arg;
}

seg_expr_node *seg_implicit_self(seg_parser_state *state)
{
  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_VAR;
  seg_symboltable_intern(state->symboltable, "self", 4, &out->child.var.varname);
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
