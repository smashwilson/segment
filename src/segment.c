#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "lexer.h"
#include "ast_printer.h"

int main(int argc, const char **argv)
{
  int res;
  struct stat istat;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input>\n", argv[0]);
    exit(1);
  }

  int ifd = open(argv[1], O_RDONLY);
  if (ifd == -1) {
    perror("unable to open input file");
    exit(1);
  }

  res = fstat(ifd, &istat);
  if (res == -1) {
    perror("unable to stat input file");
    exit(1);
  }

  void *content = mmap(NULL, (size_t) istat.st_size, PROT_READ, MAP_PRIVATE, ifd, 0);
  if (content == MAP_FAILED) {
    perror("unable to memory-map input file");
    exit(1);
  }

  printf("Beginning parse:\n");
  seg_statementlist_node *root = seg_parse((char*) content, istat.st_size);
  puts("Parse complete.\n");
  seg_print_ast(root, stdout);

  return 0;
}
