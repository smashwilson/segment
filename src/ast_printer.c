#include <stdio.h>

#include "ast_printer.h"

typedef struct {
    FILE *out;
    int depth;
} printer_state;

static void print_prefix(printer_state *pstate)
{
  for (int i = 0; i < pstate->depth - 1; i++) {
    fputs("| ", pstate->out);
  }
  if (pstate->depth > 0) {
    fputs("|-", pstate->out);
  }
}

static void print_integer(seg_integer_node *node, void *state)
{
  printer_state *pstate = (printer_state *) state;
  print_prefix(pstate);

  fprintf(pstate->out, "INTEGER: %d\n", node->value);
}

static void print_binop(seg_binop_node *node, void *state)
{
  printer_state *pstate = (printer_state *) state;
  print_prefix(pstate);

  fprintf(pstate->out, "BINOP: %s\n", node->selector);
  pstate->depth++;
}

static void print_expr(seg_expr_node *node, void *state)
{
  printer_state *pstate = (printer_state *) state;
  print_prefix(pstate);

  fputs("EXPR\n", pstate->out);
  pstate->depth++;
}

static void print_statement(seg_statement_node *node, void *state)
{
  printer_state *pstate = (printer_state *) state;
  print_prefix(pstate);

  fputs("STATEMENT\n", pstate->out);
  pstate->depth++;
}

static void print_statementlist(seg_statementlist_node *node, void *state)
{
  printer_state *pstate = (printer_state *) state;
  print_prefix(pstate);

  fputs("STATEMENTLIST\n", pstate->out);
  pstate->depth++;
}

static void pop_depth(void *ignored, void *state)
{
  printer_state *pstate = (printer_state *) state;
  pstate->depth--;
}

void seg_print_ast(seg_statementlist_node *root, FILE *outf)
{
    printer_state pstate;
    pstate.out = outf;
    pstate.depth = 0;

    seg_ast_visitor visitor = seg_new_ast_visitor();

    seg_ast_visit_integer(visitor, &print_integer);

    seg_ast_visit_binop(visitor, SEG_VISIT_PRE, &print_binop);
    seg_ast_visit_binop(visitor, SEG_VISIT_POST, (seg_binop_handler) &pop_depth);

    seg_ast_visit_expr(visitor, SEG_VISIT_PRE, &print_expr);
    seg_ast_visit_expr(visitor, SEG_VISIT_POST, (seg_expr_handler) &pop_depth);

    seg_ast_visit_statement(visitor, SEG_VISIT_PRE, &print_statement);
    seg_ast_visit_statement(visitor, SEG_VISIT_POST, (seg_statement_handler) &pop_depth);

    seg_ast_visit_statementlist(visitor, SEG_VISIT_PRE, &print_statementlist);
    seg_ast_visit_statementlist(visitor, SEG_VISIT_POST, (seg_statementlist_handler) &pop_depth);

    seg_ast_visit(visitor, root, &pstate);

    seg_delete_ast_visitor(visitor);
}
