#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "token.h"

seg_token *seg_new_token(const char *start, char *end)
{
  seg_token *tok = malloc(sizeof(seg_token));
  tok->start = start;
  tok->end = end;
  return tok;
}

char *seg_token_as_string(seg_token *tok)
{
  size_t length = tok->end - tok->start;
  char *v = malloc(length);
  strncpy(v, tok->start, length);
  return v;
}

long seg_token_as_integer(seg_token *tok)
{
  long v = strtol(tok->start, &tok->end, 10);

  if (errno) {
    perror("Unable to parse token as integer");
    return 0L;
  }

  return v;
}

void seg_delete_token(seg_token *tok)
{
  free(tok);
}
