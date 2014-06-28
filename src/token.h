#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

#include "symboltable.h"

typedef struct {
  const char *start;
  size_t length;
} seg_token;

/*
 * Allocate a new seg_token from the given start and end points.
 */
seg_token *seg_new_token(const char *start, const char *end);

/*
 * Copy this token into an independently allocated string and compute its length.
 */
char *seg_token_as_string(seg_token *tok, size_t *length);

/*
 * Intern part of the content of this token as a symbol.
 */
seg_symbol *seg_token_intern_without(
  seg_token *tok,
  seg_symboltable *table,
  size_t initial,
  size_t final
);

/*
 * Copy this token into an independently allocated string, omitted expected final and initial
 * characters, specified by bytes.
 *
 * Note that this won't work as-is for specifying a final glyph that occupies more than a single
 * byte.
 */
char *seg_token_without(seg_token *tok, size_t initial, size_t final, size_t *length);

/*
 * Interpret this token as an integer.
 */
long seg_token_as_integer(seg_token *tok);

/*
 * Destroy a token allocated by seg_new_token.
 */
void seg_delete_token(seg_token *tok);

#endif
