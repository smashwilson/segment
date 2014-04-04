#ifndef AST_H
#define AST_H

/* Kind Tag Enums */

typedef enum {
  EXPR
} seg_statement_kind;

typedef enum {
  INTEGER,
  BINOP
} seg_expr_kind;

/* Forward Declarations */

struct seg_expr_node;

/* Literals */

typedef struct {
  int value;
} seg_integer_node;

/* Invocation */

typedef struct {
  const char *selector;
  struct seg_expr_node *left;
  struct seg_expr_node *right;
} seg_binop_node;

/* Grouping Constructs */

typedef struct seg_expr_node {
  union {
    seg_integer_node *integer;
    seg_binop_node *binop;
  } expr;
  seg_expr_kind kind;
} seg_expr_node;

typedef struct seg_statement_node {
  union {
    seg_expr_node *expr;
  } child;
  seg_statement_kind child_kind;

  struct seg_statement_node *next;
} seg_statement_node;

typedef struct {
  seg_statement_node *first;
} seg_statementlist_node;

#endif
