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
