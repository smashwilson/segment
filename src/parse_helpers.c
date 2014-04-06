#include <stdlib.h>

#include "parse_helpers.h"

seg_expr_node *parse_binop(seg_expr_node *lhs, seg_token *tok, seg_expr_node *rhs)
{
  seg_binop_node *op = malloc(sizeof(seg_binop_node));
  op->selector = seg_token_as_string(tok, &(op->length));
  seg_delete_token(tok);
  op->left = lhs;
  op->right = rhs;

  seg_expr_node *out = malloc(sizeof(seg_expr_node));
  out->child_kind = SEG_BINOP;
  out->child.binop = op;
}
