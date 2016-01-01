#ifndef AST_H
#define AST_H

#include <stddef.h>

#include "model/object.h"
#include "runtime/symboltable.h"

/* Kind Tag Enums */

typedef enum {
  SEG_INTEGER,
  SEG_STRING,
  SEG_SYMBOL,
  SEG_VAR,
  SEG_BLOCK,
  SEG_METHODCALL
} seg_expr_kind;

/* Forward Declarations */

struct seg_expr_node;

/* Literals */

typedef struct {
  int64_t value;
} seg_integer_node;

typedef struct {
  const char *value;
  uint64_t length;
} seg_string_node;

typedef struct {
  seg_object value;
} seg_symbol_node;

/* Variable References */

typedef struct {
  seg_object varname;
} seg_var_node;

/* Blocks */

typedef struct seg_parameter_list {
  seg_object parameter;
  struct seg_parameter_list *next;
} seg_parameter_list;

typedef struct {
  seg_parameter_list *parameters;
  struct seg_expr_node *first;
  struct seg_expr_node *last;
} seg_block_node;

/* Method Invocation */

typedef struct seg_arg_list {
  /* Keyword will most often be left as NULL. */
  seg_object keyword;

  struct seg_expr_node *value;
  struct seg_arg_list *next;
} seg_arg_list;

typedef struct {
  struct seg_expr_node *receiver;
  seg_object selector;
  seg_arg_list *args;
} seg_methodcall_node;

/* Grouping Constructs */

typedef struct seg_expr_node {
  union {
    seg_integer_node integer;
    seg_string_node string;
    seg_symbol_node symbol;
    seg_var_node var;
    seg_block_node block;
    seg_methodcall_node methodcall;
  } child;
  seg_expr_kind child_kind;
  struct seg_expr_node *next;
} seg_expr_node;

/* Visitor */

// FIXME don't do this.
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
typedef void (*seg_string_handler)(seg_string_node *node, void *state);
typedef void (*seg_symbol_handler)(seg_symbol_node *node, void *state);
typedef void (*seg_methodcall_handler)(seg_methodcall_node *node, void *state);
typedef void (*seg_var_handler)(seg_var_node *node, void *state);
typedef void (*seg_block_handler)(seg_block_node *node, void *state);

seg_ast_visitor seg_new_ast_visitor();

void seg_ast_visit_integer(seg_ast_visitor visitor, seg_integer_handler visit);
void seg_ast_visit_string(seg_ast_visitor visitor, seg_string_handler visit);
void seg_ast_visit_symbol(seg_ast_visitor visitor, seg_symbol_handler visit);
void seg_ast_visit_methodcall(
  seg_ast_visitor visitor,
  seg_visit_when when,
  seg_methodcall_handler visit
);
void seg_ast_visit_var(seg_ast_visitor visitor, seg_var_handler);
void seg_ast_visit_block(seg_ast_visitor visitor, seg_visit_when, seg_block_handler visit);

void seg_ast_visit(
  seg_ast_visitor visitor,
  seg_block_node *root,
  void *state
);

void seg_delete_ast_visitor(seg_ast_visitor visitor);

#endif
