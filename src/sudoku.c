#include "sudoku.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <err.h>
#include <getopt.h>

#include <grid.h>

#define DEFAULT_GRID_SIZE 9

static bool verbose = false;

/* returns NULL when parsing fails, the parsed grid pointer otherwise */
static grid_t* file_parser(char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    warnx("File %s cannot open", filename);
    return NULL;
  }

  char first_row[MAX_GRID_SIZE] = {0};
  bool in_comment = false;
  int c;
  size_t grid_size = 0;
  int line_counter = 1;
  bool found_any_content = false;

  /* Read until first valid character to determine grid size */
  while ((c = fgetc(file)) != EOF && grid_size < MAX_GRID_SIZE) {
    switch (c) {
      case '#':
        while ((c = fgetc(file)) != EOF && c != '\n')
          continue;

        if (c == '\n')
          line_counter++;
        continue;

      case '\n':
        if (!found_any_content) {
          line_counter++;
          continue;
        }
        goto quit_loop_first_line;

      case '\t':
      case ' ':
        continue;
    }

    found_any_content = true;
    first_row[grid_size++] = (char)c;
  }

quit_loop_first_line:
  if (!found_any_content) {
    fclose(file);
    return NULL;
  }

  /* Validate detected grid size */
  if (!grid_check_size(grid_size)) {
    fclose(file);
    warnx("warning: '%s': invalid grid size '%zu' (valid sizes are: "
          "1,4,9,16,25,36,49,64)",
          filename,
          grid_size);
    return NULL;
  }

  grid_t* grid = grid_alloc(grid_size);
  if (!grid) {
    fclose(file);
    warnx("error in memory allocation while parsing the file : %s", filename);
  }

  /* Validate first row characters */
  for (size_t i = 0; i < grid_size; i++) {
    if (!grid_check_char(grid, first_row[i])) {
      fclose(file);
      grid_free(grid);
      warnx("warning: '%s': wrong character '%c' at line %d!",
            filename,
            first_row[i],
            line_counter);
      return NULL;
    }

    grid_set_cell(grid, 0, i, first_row[i]);
  }

  size_t row_counter = 1;
  size_t col_counter = 0;
  in_comment = false;

  /* Parse remaining grid lines using grid size */
  while (row_counter < grid_size) {
    c = fgetc(file);
    switch (c) {
      case EOF:
        if (col_counter == 0)
          goto quit_loop_grid_end;

        if (col_counter != grid_size) {
          fclose(file);
          grid_free(grid);
          warn("warning: '%s': line %d is malformed! "
               "(wrong number of columns)",
               filename,
               line_counter);
          return NULL;
        }

        row_counter++;
        goto quit_loop_grid_end;

      case '#':
        in_comment = true;
        continue;

      case '\n':
        if (in_comment) {
          in_comment = false;
          line_counter++;
          continue;
        }

        if (col_counter > 0 && col_counter != grid_size) {
          fclose(file);
          grid_free(grid);
          warnx("warning: '%s': line %d is malformed! "
                "(wrong number of columns)",
                filename,
                line_counter);
          return NULL;
        }

        if (col_counter > 0) {
          row_counter++;
          col_counter = 0;
        }

        line_counter++;
        continue;

      default:
        if (in_comment || isspace(c)) {
          continue;
        }

        if (!grid_check_char(grid, (char)c)) {
          fclose(file);
          grid_free(grid);
          warnx("warning: '%s': wrong character '%c' at line %d!",
                filename,
                (char)c,
                line_counter);
          return NULL;
        }

        if (col_counter >= grid_size) {
          fclose(file);
          grid_free(grid);
          warnx("warning: '%s': line %d is malformed! (too many columns)",
                filename,
                line_counter);
          return NULL;
        }

        grid_set_cell(grid, row_counter, col_counter, (char)c);
        col_counter++;
        break;
    }
  }

quit_loop_grid_end:
  if (row_counter != grid_size) {
    fclose(file);
    grid_free(grid);
    if (row_counter < grid_size) {
      warnx("warning: '%s': grid has %zu missing line(s)!",
            filename,
            grid_size - row_counter);
      return NULL;
    }

    warnx("warning: '%s': grid has too many lines!", filename);
    return NULL;
  }

  /* Detect extra content after the grid */
  bool has_extra_content = false;
  while ((c = fgetc(file)) != EOF) {
    switch (c) {
      case '#':
        while ((c = fgetc(file)) != EOF && c != '\n')
          continue;
        if (c == '\n')
          line_counter++;
        continue;

      case ' ':
      case '\r':
      case '\t':
        continue;

      case '\n':
        line_counter++;
        continue;

      default:
        has_extra_content = true;
        goto quit_loop_extra_lines;
    }
  }

quit_loop_extra_lines:
  if (has_extra_content) {
    fclose(file);
    grid_free(grid);
    warnx("warning: '%s': grid has extra line(s)!", filename);
    return NULL;
  }

  fclose(file);
  return grid;
}

int main(int argc, char* argv[]) {
  opterr = 0; /* Disable getopt_long error messages */
  int optc;
  bool all = false;
  bool unique = false;
  bool solver = true;
  bool generator = false;
  FILE* outfile = stdout;
  char* outfile_name = NULL;
  int grid_size = DEFAULT_GRID_SIZE;
  const char* args = ":ao:hvuVg::";

  /* Long options */
  struct option const long_opts[] = {{"all", no_argument, NULL, 'a'},
                                     {"help", no_argument, NULL, 'h'},
                                     {"verbose", no_argument, NULL, 'v'},
                                     {"unique", no_argument, NULL, 'u'},
                                     {"generate", optional_argument, NULL, 'g'},
                                     {"version", no_argument, NULL, 'V'},
                                     {"output", required_argument, NULL, 'o'},
                                     {NULL, 0, NULL, 0}};

  /* Parsing options */
  while ((optc = getopt_long(argc, argv, args, long_opts, NULL)) != -1) {
    switch (optc) {
      /* Verbose option */
      case 'v':
        verbose = true;
        printf("the option 'verbose' is set\n");
        break;

      /* All option */
      case 'a':
        all = true;
        break;

      /* Unique option */
      case 'u':
        unique = true;
        break;

      /* Version option */
      case 'V':
        if (outfile && outfile != stdout)
          fclose(outfile);
        printf("sudoku %d.%d.%d\n", VERSION, SUBVERSION, REVISION);
        fputs("Solve/generate sudoku grids of size: 1, 4, 9, 16, 25, 36, 49, "
              "64\n",
              stdout);
        exit(EXIT_SUCCESS);

      /* Help option */
      case 'h':
        if (outfile && outfile != stdout)
          fclose(outfile);
        fputs("Usage: sudoku [-a|-o FILE|-v|-V|-h] FILE...\n"
              "       sudoku -g[SIZE] [-u|-o FILE|-v|-V|-h]\n"
              "Solve or generate Sudoku grids of size: 1, 4, 9, 16, 25, 36, "
              "49, 64\n\n"
              " -a,--all                 search for all possible solutions\n"
              " -g[N],--generate[=N]     generate a grid of size NxN "
              "(default:9)\n"
              " -o FILE,--output FILE    write output to FILE\n"
              " -u,--unique              generate a grid with unique solution\n"
              " -v,--verbose             verbose output\n"
              " -V,--version             display version and exit\n"
              " -h,--help                display this help and exit\n",
              stdout);
        exit(EXIT_SUCCESS);

      /* Generator mode */
      case 'g':
        if (optarg) {
          char* endptr;
          grid_size = strtol(optarg, &endptr, 10);
          if (!grid_check_size(grid_size) || *endptr != '\0') {
            errx(EXIT_FAILURE,
                 "error: invalid grid size '%s': (valid sizes are: "
                 "1,4,9,16,25,36,49,64)",
                 optarg);
          }
        }
        printf("Generating grid of size %d\n", grid_size);
        generator = true;
        solver = false;
        break;

      /* Output option */
      case 'o':
        outfile_name = optarg;
        break;

      /* Missing option argument */
      case ':':
        errx(EXIT_FAILURE,
             "error: option '%s': requires an argument",
             argv[optind - 1]);

      /* Unknown option */
      default:
        errx(EXIT_FAILURE, "error: invalid option '%s':!", argv[optind - 1]);
    }
  }

  /* Handle outputs and conflicts with generator mode */
  if (generator) {
    if (all) {
      warnx(
          "warning: option 'all' conflicts with generator mode, disabling it");
      all = false;
    }

    if (unique) {
      printf("the option 'unique' is set\n");
    }
  }

  /* Handle outputs and conflicts with solver mode */
  if (solver) {
    if (optind >= argc) {
      if (outfile)
        fclose(outfile);
      errx(EXIT_FAILURE, "error: no input grid given!");
    }

    if (outfile_name != NULL) {
      outfile = fopen(outfile_name, "w+");

      if (!outfile) {
        errx(
            EXIT_FAILURE, "error: cannot open output file '%s':", outfile_name);
      }
    }

    if (unique) {
      warnx("warning: the option 'unique' conflicts with solver mode, "
            "disabling it");
      printf("solving a grid\n");
      unique = false;
    }

    if (all) {
      printf("the option 'all' is set\n");
    }

    bool found_inconsistent = false;
    bool found_bad_file = false;
    solver_mode_t mode = all ? mode_all : mode_first;

    for (int i = optind; i < argc; i++) {
      grid_t* grid = file_parser(argv[i]);
      if (!grid) {
        found_bad_file = true;
        continue;
      }

      fprintf(outfile, "Solving %s...\n\n", argv[i]);

      status_t grid_status;
      int counter = 0;
      bool verbose = false;
      grid_t* solved_grid = grid_solver(grid, mode, verbose, &counter, outfile);

      if (all) {
        if (counter != 0)
          fprintf(outfile, "Number of found solutions: %d\n", counter);
        grid_status = (counter > 0) ? grid_solved : grid_inconsistent;
      } else {
        if (solved_grid) {
          if (solved_grid != grid)
            grid_free(solved_grid);
          grid_status = grid_solved;
        } else {
          grid_status = grid_inconsistent;
        }
      }

      switch (grid_status) {
        case grid_solved:
        case grid_unsolved:
          break;

        case grid_inconsistent:
          fprintf(outfile, "The grid '%s' is inconsistent!\n", argv[i]);
          found_inconsistent = true;
          break;
      }

      grid_free(grid);
    }

    if (outfile && outfile != stdout)
      fclose(outfile);

    if (found_inconsistent)
      errx(EXIT_FAILURE, "Some inconsistent grids were found!");

    if (found_bad_file)
      errx(EXIT_FAILURE, "Some malformed grids were found!");
  }

  return EXIT_SUCCESS;
}
