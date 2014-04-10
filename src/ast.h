#ifndef AST_H
#define AST_H

#include <stddef.h>

/* Kind Tag Enums */

typedef enum {
  SEG_INTEGER,
  SEG_VAR,
  SEG_BLOCK,
  SEG_METHODCALL
} seg_expr_kind;

/* Forward Declarations */

struct seg_expr_node;

struct seg_statementlist_node;

/* Literals */

typedef struct {
  int value;
} seg_integer_node;

/* Variable References */

typedef struct {
  const char *varname;
  size_t length;
} seg_var_node;

/* Blocks */

typedef struct seg_parameter_list {
  const char *name;
  size_t length;
  struct seg_parameter_list *next;
} seg_parameter_list;

typedef struct {
  seg_parameter_list *parameters;
  struct seg_statementlist_node *body;
} seg_block_node;

/* Method Invocation */

typedef struct seg_arg_list {
  /* Keyword will most often be left as NULL. */
  const char *keyword;
  size_t length;

  struct seg_expr_node *value;
  struct seg_arg_list *next;
} seg_arg_list;

typedef struct {
  struct seg_expr_node *receiver;
  const char *selector;
  size_t length;
  seg_arg_list *args;
} seg_methodcall_node;

/* Grouping Constructs */

typedef struct seg_expr_node {
  union {
    seg_integer_node *integer;
    seg_var_node *var;
    seg_block_node *block;
    seg_methodcall_node *methodcall;
  } child;
  seg_expr_kind child_kind;
  struct seg_expr_node *next;
} seg_expr_node;

typedef struct seg_statementlist_node {
  seg_expr_node *first;
  seg_expr_node *last;
} seg_statementlist_node;

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
typedef void (*seg_methodcall_handler)(seg_methodcall_node *node, void *state);
typedef void (*seg_var_handler)(seg_var_node *node, void *state);
typedef void (*seg_block_handler)(seg_block_node *node, void *state);
typedef void (*seg_expr_handler)(seg_expr_node *node, void *state);
typedef void (*seg_statementlist_handler)(seg_statementlist_node *node, void *state);

seg_ast_visitor seg_new_ast_visitor();

void seg_ast_visit_integer(seg_ast_visitor visitor, seg_integer_handler visit);
void seg_ast_visit_methodcall(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_methodcall_handler visit
);
void seg_ast_visit_var(seg_ast_visitor visitor, seg_var_handler);
void seg_ast_visit_block(seg_ast_visitor visitor, seg_visit_when, seg_block_handler visit);
void seg_ast_visit_expr(seg_ast_visitor visitor, seg_visit_when when, seg_expr_handler visit);

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
