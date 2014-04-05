#ifndef SEGMENT_H
#define SEGMENT_H

typedef struct {
  int lexer_debug;
  int ast_debug;
  int verbose;

  const char **src_paths;
  int src_count;
} seg_options;

#endif
