#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "segment.h"
#include "lexer.h"
#include "ast_printer.h"

/*
 * Print a usage statement and exit with an exit code.
 */
static void print_usage(FILE *dest, int code, const char *progname)
{
  fprintf(dest, "Usage: %s [--lexer-debug] [--ast-debug] [--verbose|-v] file ...\n", progname);
  fprintf(dest, "\n  --lexer-debug  Output each token the lexer identifies.\n");
  fprintf(dest, "\n  --ast-debug    Write an abstract syntax tree in ASCII-art.\n");
  fprintf(dest, "\n  --verbose      Output banners and status.\n");
  fprintf(dest, "\n  file           Interpret each file in sequence.\n");
  exit(code);
}

/*
 * Interpret command-line options.
 */
static void process_options(int argc, char **argv, seg_options *opts)
{
  int c = 0;
  int option_index = 0;

  static int lexer_debug = 0;
  static int ast_debug = 0;

  static struct option long_options [] = {
    {"lexer-debug", no_argument, &lexer_debug, 1},
    {"ast-debug", no_argument, &ast_debug, 1},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
  };

  /* Set defaults. */
  opts->src_paths = NULL;
  opts->src_count = 0;
  opts->verbose = 0;

  while (c != -1) {
    c = getopt_long(argc, argv, "hv", long_options, &option_index);

    switch (c) {
      case 'h':
        print_usage(stdout, 0, argv[0]);
        break;
      case 'v':
        opts->verbose = 1;
        break;
      case '?':
        print_usage(stderr, 1, argv[0]);
        break;
    }
  }

  opts->lexer_debug = lexer_debug;
  opts->ast_debug = ast_debug;

  if (optind < argc) {
    int count = argc - optind;
    opts->src_count = count;
    opts->src_paths = malloc(sizeof(char*) * count);

    for (int i = optind; i < argc; i++) {
      opts->src_paths[i - optind] = argv[i];
    }
  }

  /* Validate argument state. */
  if (opts->src_count <= 0) {
    fprintf(stderr, "Please provide at least one source file.\n\n");
    /* ... at least until I implement a proper repl ;-) */

    print_usage(stderr, 1, argv[0]);
  }
}

static int process_file(const char *path, seg_options *opts)
{
  int res;
  struct stat istat;

  int ifd = open(path, O_RDONLY);
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

  seg_program *program = seg_parse((char*) content, istat.st_size, opts);

  if (program->ast == NULL) {
    fputs("Syntax error!", stderr);
    return 1;
  }

  if (opts->ast_debug) {
    if (opts->verbose && opts->lexer_debug) {
      putchar('\n');
    }

    if (opts->verbose) {
      puts("Abstract syntax tree:\n");
    }

    seg_print_ast(program->ast, stdout);
  }

  return 0;
}

int main(int argc, char **argv)
{
  int res;
  seg_options opts;

  process_options(argc, argv, &opts);

  for (int i = 0; i < opts.src_count; i++) {
    res = process_file(opts.src_paths[i], &opts);
    if (res) {
      return res;
    }
  }

  return 0;
}
