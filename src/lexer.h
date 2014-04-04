#ifndef LEXER_H
#define LEXER_H

#include <sys/types.h>

/**
 * Parse a coherent chunk of segment code and return its AST.
 */
int seg_parse(char *content, off_t length);

#endif
