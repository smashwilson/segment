#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "token.h"
#include "runtime/symboltable.h"

seg_token *seg_new_token(const char *start, const char *end)
{
  seg_token *tok = malloc(sizeof(seg_token));
  tok->start = start;
  tok->length = end - start;
  return tok;
}

char *seg_token_as_string(seg_token *tok, size_t *length)
{
  char *v = malloc(tok->length);
  strncpy(v, tok->start, tok->length);
  *length = tok->length;
  return v;
}

seg_object seg_token_intern_without(
  seg_token *tok,
  seg_symboltable *table,
  size_t initial,
  size_t final
) {
  size_t len = tok->length - initial - final;

  seg_object symbol;
  seg_symboltable_intern(table, tok->start + initial, len, &symbol);
  return symbol;
}

char *seg_token_without(seg_token *tok, size_t initial, size_t final, size_t *length)
{
  size_t len = tok->length - initial - final;
  *length = len;

  if (len == 0) {
    return NULL;
  }

  char *v = malloc(len);
  strncpy(v, tok->start + initial, len);
  return v;
}

long seg_token_as_integer(seg_token *tok)
{
  char *end = (char*) 0x1;
  long v = strtol(tok->start, &end, 10);

  if (errno) {
    perror("Unable to parse token as integer");
    return 0L;
  }

  if (end != tok->start + tok->length) {
    fprintf(stderr, "Warning: INTEGER had unexpected length\n");
  }

  return v;
}

void seg_delete_token(seg_token *tok)
{
  free(tok);
}
