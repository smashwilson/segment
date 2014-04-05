#ifndef AST_H
#define AST_H

/* Kind Tag Enums */

typedef enum {
  SEG_INTEGER,
  SEG_BINOP,
  SEG_BLOCK
} seg_expr_kind;

/* Forward Declarations */

struct seg_expr_node;

struct seg_statementlist_node;

/* Literals */

typedef struct {
  int value;
} seg_integer_node;

/* Blocks */

typedef struct seg_parameter_list {
  const char *name;
  struct seg_parameter_list *next;
} seg_parameter_list;

typedef struct {
  seg_parameter_list *parameters;
  struct seg_statementlist_node *body;
} seg_block_node;

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
    seg_block_node *block;
  } child;
  seg_expr_kind child_kind;
  struct seg_expr_node *next;
} seg_expr_node;

typedef struct seg_statementlist_node {
  seg_expr_node *first;
  seg_expr_node *last;
} seg_statementlist_node;

typedef struct {
  seg_statementlist_node *root;
} seg_program_node;

/* Visitor */

typedef struct seg_ast_visitor* seg_ast_visitor;

/*
  Determine if a visit handler is being invoked before or after any child nodes
  have been visited.
 */
typedef enum {
  SEG_VISIT_PRE,
  SEG_VISIT_POST
} seg_visit_when;

typedef void (*seg_integer_handler)(seg_integer_node *node, void *state);
typedef void (*seg_binop_handler)(seg_binop_node *node, void *state);
typedef void (*seg_block_handler)(seg_block_node *node, void *state);
typedef void (*seg_expr_handler)(seg_expr_node *node, void *state);
typedef void (*seg_statementlist_handler)(seg_statementlist_node *node, void *state);

seg_ast_visitor seg_new_ast_visitor();

void seg_ast_visit_integer(
  seg_ast_visitor visitor,
  seg_integer_handler visit
);

void seg_ast_visit_binop(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_binop_handler visit
);

void seg_ast_visit_block(
  seg_ast_visitor visitor,
  seg_visit_when,
  seg_block_handler visit
);

void seg_ast_visit_expr(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_expr_handler visit
);

void seg_ast_visit_statementlist(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_statementlist_handler visit
);

void seg_ast_visit(
  seg_ast_visitor visitor,
  seg_statementlist_node *root,
  void *state
);

void seg_delete_ast_visitor(seg_ast_visitor visitor);

#endif
