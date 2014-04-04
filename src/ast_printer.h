#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast.h"

/*
 Print an ASCII-art representation of a parsed AST.

 Mostly useful for debugging the parser itself.
 */
void seg_print_ast(seg_statementlist_node *root, FILE *outf);

#endif
