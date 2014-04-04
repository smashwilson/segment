#ifndef AST_H
#define AST_H

/* Kind Tag Enums */

typedef enum {
  SEG_EXPR
} seg_statement_kind;

typedef enum {
  SEG_INTEGER,
  SEG_BINOP
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

typedef void (*seg_integer_handler)(seg_integer_node *node, seg_visit_when when, void *state);
typedef void (*seg_binop_handler)(seg_binop_node *node, seg_visit_when when, void *state);
typedef void (*seg_expr_handler)(seg_expr_node *node, seg_visit_when when, void *state);
typedef void (*seg_statement_handler)(seg_statement_node *node, seg_visit_when when, void *state);
typedef void (*seg_statementlist_handler)(seg_statementlist_node *node, seg_visit_when when, void *state);

seg_ast_visitor seg_new_ast_visitor();

void seg_ast_visit_integer(seg_ast_visitor visitor, seg_integer_handler visit);
void seg_ast_visit_binop(seg_ast_visitor visitor, seg_binop_handler visit);
void seg_ast_visit_expr(seg_ast_visitor visitor, seg_expr_handler visit);
void seg_ast_visit_statement(seg_ast_visitor visitor, seg_statement_handler visit);
void seg_ast_visit_statementlist(seg_ast_visitor visitor, seg_statementlist_handler visit);

void seg_ast_visit(
  seg_ast_visitor visitor,
  seg_statementlist_node *root,
  void *state
);

void seg_delete_ast_visitor(seg_ast_visitor visitor);

#endif
