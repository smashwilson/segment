#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct {
  int lexer_debug;

  int ast_invoke;
  int ast_debug;

  int symbol_debug;

  int verbose;

  const char **src_paths;
  int src_count;
} seg_options;

#endif
