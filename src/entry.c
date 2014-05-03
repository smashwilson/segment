#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "options.h"
#include "lexer.h"
#include "debug/ast_printer.h"
#include "debug/symbol_printer.h"

/*
 * Print a usage statement and exit with an exit code.
 */
static void print_usage(FILE *dest, int code, const char *progname)
{
  fprintf(
    dest,
    "Usage: %s [--debug lexer|ast|symbol] [--phase lexer|ast] [--verbose|-v] "
    "file ...\n",
    progname);
  fprintf(dest, "\n  --debug PHASE  Produce debugging output for the specified phase.\n");
  fprintf(dest, "  --phase PHASE  Execute only up to the specified phase.\n");
  fprintf(dest, "  --verbose      Output banners and statistics.\n");
  fprintf(dest, "  file           Interpret each file in sequence.\n");
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
    {"debug", required_argument, NULL, 'd'},
    {"phase", required_argument, NULL, 'p'},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
  };

  /* Set defaults. */
  opts->src_paths = NULL;
  opts->src_count = 0;
  opts->verbose = 0;

  opts->lexer_debug = 0;

  opts->ast_invoke = 1;
  opts->ast_debug = 0;

  opts->symbol_debug = 0;

  while (c != -1) {
    c = getopt_long(argc, argv, "d:p:hv", long_options, &option_index);

    switch (c) {
      case 'd':
        if (! strncmp(optarg, "lexer", 6)) {
          opts->lexer_debug = 1;
        } else if (! strncmp(optarg, "ast", 4)) {
          opts->ast_debug = 1;
        } else if (! strncmp(optarg, "symbol", 7)) {
          opts->symbol_debug = 1;
        } else {
          fprintf(stderr, "segment: Unrecognized --debug phase <%s>.\n", optarg);
          fprintf(stderr, "segment: Available phases are: lexer, ast, symbol.\n");
          print_usage(stderr, 1, argv[0]);
        }
        break;
      case 'p':
        if (! strncmp(optarg, "lexer", 6)) {
          opts->ast_invoke = 0;
        } else if (! strncmp(optarg, "ast", 4)) {
          opts->ast_invoke = 1;
        } else {
          fprintf(stderr, "segment: Unrecognized --phase <%s>.\n", optarg);
          fprintf(stderr, "segment: Available phases are: lexer, ast.\n");
          print_usage(stderr, 1, argv[0]);
        }
        break;
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

  if (program->ast == NULL && opts->ast_invoke) {
    fputs("Syntax error!\n", stderr);
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

  if (opts->symbol_debug) {
    if (opts->verbose && (opts->lexer_debug || opts->ast_debug)) {
      putchar('\n');
    }

    if (opts->verbose) {
      puts("Symbol table contents:\n");
    }

    seg_print_symboltable(program->symboltable);
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
