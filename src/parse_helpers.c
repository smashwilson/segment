#include <stdlib.h>

#include "parse_helpers.h"
#include "ast.h"

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
