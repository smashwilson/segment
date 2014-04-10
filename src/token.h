#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

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
 * Copy this token into an independently allocated string, omitted an expected final character.
 * If the expected character is missing, a warning will be printed and the full value will be
 * returned instead.
 *
 * Note that this won't work as-is for specifying a final glyph that occupies more than a single
 * byte.
 */
char *seg_token_without(seg_token *tok, size_t *length, char final);

/*
 * Interpret this token as an integer.
 */
long seg_token_as_integer(seg_token *tok);

/*
 * Destroy a token allocated by seg_new_token.
 */
void seg_delete_token(seg_token *tok);

#endif
