#ifndef TOKEN_H
#define TOKEN_H

typedef struct {
  const char *start;
  char *end;
} seg_token;

/*
 * Allocate a new seg_token from the given start and end points.
 */
seg_token *seg_new_token(const char *start, char *end);

/*
 * Copy this token into a null-terminated string.
 */
const char *seg_token_as_string(seg_token *tok);

/*
 * Interpret this token as an integer.
 */
long seg_token_as_integer(seg_token *tok);

/*
 * Destroy a token allocated by seg_new_token.
 */
void seg_delete_token(seg_token *tok);

#endif
