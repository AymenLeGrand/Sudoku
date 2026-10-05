#include "grid.h"

#include <string.h>
#include <time.h>

#include <colors.h>

/* Internal structure (hidden from outside) for a sudoku grid */
struct _grid_t {
  size_t size;
  colors_t** cells;
};

grid_t* grid_alloc(size_t size) {
  if (!grid_check_size(size))
    return NULL;

  grid_t* grid = malloc(sizeof(grid_t));
  if (!grid)
    return NULL;

  colors_t** cells = malloc(sizeof(colors_t*) * size);
  if (!cells) {
    free(grid);
    return NULL;
  }

  for (size_t i = 0; i < size; i++) {
    cells[i] = malloc(sizeof(colors_t) * size);
    if (!cells[i]) {
      for (size_t j = 0; j < i; j++) {
        free(cells[j]);
      }
      free(cells);
      free(grid);

      return NULL;
    }
  }

  grid->cells = cells;
  grid->size = size;
  return grid;
}

void grid_free(grid_t* grid) {
  if (!grid)
    return;

  for (size_t i = 0; i < grid->size; i++) {
    free(grid->cells[i]);
  }
  free(grid->cells);
  free(grid);
}

size_t grid_print(const grid_t* grid, FILE* fd) {
  if (!grid || !fd)
    return 0;

  size_t size = grid_get_size(grid);
  size_t num_chars = 0;
  size_t max_length = 0;
  size_t secondary_max = 0;

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      char* cell_string = grid_get_cell(grid, i, j);
      size_t cell_string_length = strlen(cell_string);

      if (cell_string_length > max_length)
        max_length = cell_string_length;

      if (cell_string_length > secondary_max && cell_string_length < size)
        secondary_max = cell_string_length;

      free(cell_string);
    }
  }

  size_t padding_max = 0;
  if (max_length == 1) {
    padding_max = 0;
  } else if (max_length == size) {
    padding_max = (secondary_max > 1) ? secondary_max : 0;
  } else {
    padding_max = max_length;
  }

  int apply_padding = (padding_max > 0);

  /* Special case for grid of size 01x01 */
  if (size == 1) {
    num_chars += fprintf(fd, "%c\n", '1');
    return num_chars;
  }

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      char* cell_string = grid_get_cell(grid, i, j);
      const char* printed_string = cell_string;

      if (strlen(cell_string) == size) {
        char empty_str[2] = {EMPTY_CELL, '\0'};
        printed_string = empty_str;
      }

      if (strlen(cell_string) == 0) {
        char empty_str[2] = {' ', '\0'};
        printed_string = empty_str;
      }

      if (apply_padding) {
        num_chars += fprintf(fd, "%*s", (int)padding_max, printed_string);
      } else {
        num_chars += fprintf(fd, "%s", printed_string);
      }

      free(cell_string);

      if (j < size - 1)
        num_chars += fprintf(fd, " ");
    }
    num_chars += fprintf(fd, "\n");
  }

  return num_chars;
}

bool grid_check_char(const grid_t* grid, const char c) {
  int size = grid_get_size(grid);
  switch (size) {
    case 64:
      if ((c > 'm' && c <= 'z') || c == '*' || c == '&')
        return true;
      /* FALLTHROUGH */

    case 49:
      if (c >= 'a' && c <= 'm')
        return true;
      /* FALLTHROUGH */

    case 36:
      if ((c > 'P' && c <= 'Z') || c == '@')
        return true;
      /* FALLTHROUGH */

    case 25:
      if (c > 'G' && c <= 'P')
        return true;
      /* FALLTHROUGH */

    case 16:
      if (c >= 'A' && c <= 'G')
        return true;
      /* FALLTHROUGH */

    case 9:
      if (c > '4' && c <= '9')
        return true;
      /* FALLTHROUGH */

    case 4:
      if (c > '1' && c <= '4')
        return true;
      /* FALLTHROUGH */

    case 1:
      if (c == '1' || c == EMPTY_CELL)
        return true;
      /* FALLTHROUGH */

    default:
      return false;
  }
}

bool grid_check_size(const size_t size) {
  return (size == 1 || size == 4 || size == 9 || size == 16 || size == 25 ||
          size == 36 || size == 49 || size == 64);
}

grid_t* grid_copy(const grid_t* grid) {
  size_t size = grid_get_size(grid);
  grid_t* copy = grid_alloc(size);

  if (!copy)
    return NULL;

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      copy->cells[i][j] = grid->cells[i][j];
    }
  }

  return copy;
}

/*
 @brief Converts a character representing a color into its corresponding color
 bitmask.
 @param color The character representing a color (must exist in `color_table`).
 @param size The number of available colors (i.e., the size of `color_table`).
 @return A `colors_t` bitmask with the corresponding bit set if found,
 or `colors_full(size)` if the color is not found.
 */
static colors_t char_to_color(char color, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (color_table[i] == color)
      return colors_set(i);
  }

  return colors_full(size);
}

/*
 @brief Converts a `colors_t` bitmask into its string representation.
 @param colors The bitmask representing one or more colors.
 @return A dynamically allocated string containing characters for active colors.
 Returns NULL on allocation failure.
 */
static char* colors_to_string(colors_t colors) {
  char* colors_string;

  colors_t colors_copy = colors;
  size_t index_counter = 0;
  size_t table_index = 0;

  colors_string = malloc(sizeof(char) * colors_count(colors) + 1);
  if (!colors_string)
    return NULL;

  while (colors_copy) {
    if (colors_copy & 1)
      colors_string[index_counter++] = color_table[table_index];

    colors_copy >>= 1;
    table_index++;
  }
  colors_string[index_counter] = '\0';

  return colors_string;
}

char* grid_get_cell(const grid_t* grid, const size_t row, const size_t column) {
  if (!grid)
    return NULL;

  size_t size = grid_get_size(grid);

  if (!size || row >= size || column >= size)
    return NULL;

  return colors_to_string(grid->cells[row][column]);
}

void grid_set_cell(grid_t* grid,
                   const size_t row,
                   const size_t column,
                   const char color) {
  if (grid && row < grid_get_size(grid) && column < grid_get_size(grid))
    grid->cells[row][column] = char_to_color(color, grid_get_size(grid));
}

size_t grid_get_size(const grid_t* grid) {
  if (!grid)
    return 0;
  return grid->size;
}

bool grid_is_solved(grid_t* grid) {
  if (!grid)
    return false;

  size_t size = grid->size;
  for (size_t j = 0; j < size; j++) {
    for (size_t i = 0; i < size; i++) {
      if (!colors_is_singleton(grid->cells[i][j]))
        return false;
    }
  }

  return true;
}

/*
 @brief Applies a given function to every row, column, and block of a grid.

 Iterates over all rows, columns, and square blocks (subgrids) of the grid.
 Rows and columns are processed first, followed by square blocks of size
 determined by the grid
 @param grid Pointer to the grid structure to operate on. Must not be NULL.
 @param func Function pointer to a callback that takes a subgrid array and
             its size, returning a boolean indicating success (true) or
             failure (false).

 @return true if `func` returns true for all rows, columns, and blocks; false
         if `grid` is NULL, or if `func` returns false for any subgrid.
*/
static bool subgrid_apply(grid_t* grid,
                          bool (*func)(colors_t* subgrid[],
                                       const size_t size)) {
  if (!grid)
    return false;

  size_t size = grid_get_size(grid);
  size_t block_size = 0;

  switch (size) {
    case 1:
      block_size = 1;
      break;

    case 4:
      block_size = 2;
      break;

    case 9:
      block_size = 3;
      break;

    case 16:
      block_size = 4;
      break;

    case 25:
      block_size = 5;
      break;

    case 36:
      block_size = 6;
      break;

    case 49:
      block_size = 7;
      break;

    case 64:
      block_size = 8;
      break;
  }

  colors_t* subgrid[grid->size];
  bool func_check = false;

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      subgrid[j] = &grid->cells[i][j];
    }

    func_check |= func(subgrid, grid->size);
  }

  for (size_t j = 0; j < size; j++) {
    for (size_t i = 0; i < size; i++) {
      subgrid[i] = &grid->cells[i][j];
    }

    func_check |= func(subgrid, grid->size);
  }

  int counter;

  for (size_t bi = 0; bi < size; bi += block_size) {
    for (size_t bj = 0; bj < size; bj += block_size) {
      counter = 0;
      for (size_t i = bi; i < bi + block_size; i++) {
        for (size_t j = bj; j < bj + block_size; j++) {
          subgrid[counter++] = &grid->cells[i][j];
        }
      }

      func_check |= func(subgrid, grid->size);
    }
  }

  return func_check;
}

bool grid_is_consistent(grid_t* grid) {
  if (!grid || !grid_check_size(grid->size))
    return false;

  return !subgrid_apply(grid, subgrid_inconsistency);
}

status_t grid_heuristics(grid_t* grid) {
  if (!grid)
    return grid_inconsistent;

  while (subgrid_apply(grid, subgrid_heuristics))
    continue;

  if (!grid_is_consistent(grid))
    return grid_inconsistent;

  if (grid_is_solved(grid))
    return grid_solved;

  return grid_unsolved;
}

/*
@brief Checks if a given choice is empty (i.e., has no color assigned).
@param choice The choice_t structure to check
@return true if the choice is empty, false otherwise
*/
static bool grid_choice_is_empty(const choice_t choice) {
  return colors_is_equal(choice.color, colors_empty());
}

/*
@brief Applies a given choice to the sudoku grid.
@param grid A pointer to the grid_t structure
@param choice The choice_t structure to apply
*/
static void grid_choice_apply(grid_t* grid, const choice_t choice) {
  if (!grid || choice.row >= grid->size || choice.column >= grid->size)
    return;

  grid->cells[choice.row][choice.column] = choice.color;
}

/*
@brief Discards a given choice from the sudoku grid.
@param grid A pointer to the grid_t structure
@param choice The choice_t structure to discard
*/
static void grid_choice_discard(grid_t* grid, const choice_t choice) {
  if (!grid || choice.row >= grid->size || choice.column >= grid->size)
    return;

  grid->cells[choice.row][choice.column] =
      colors_subtract(grid->cells[choice.row][choice.column], choice.color);
}

/*
@brief Prints a given choice to the specified file descriptor.
@param choice The choice_t structure to print
@param fd The file descriptor to print the choice to
*/
static void grid_choice_print(const choice_t choice, FILE* fd) {
  if (fd)
    fprintf(fd,
            "The choice at [%ld][%ld] is: %s\n",
            choice.row,
            choice.column,
            colors_to_string(choice.color));
}

/* Minimum Remaining Values (MRV) heuristic: select unsolved cell with smallest candidate set */
static choice_t grid_choice(grid_t* grid) {
  if (!grid)
    return (choice_t){0, 0, colors_empty()};

  size_t min_count = grid->size;
  size_t row = 0;
  size_t col = 0;

  for (size_t i = 0; i < grid->size; i++) {
    for (size_t j = 0; j < grid->size; j++) {
      size_t count = colors_count(grid->cells[i][j]);

      if (count > 1 && count < min_count) {
        min_count = count;
        row = i;
        col = j;

        /* Optimal early exit: cell with only 2 candidate colors */
        if (count == 2)
          break;
      }
    }
  }

  if (min_count == grid->size)
    return (choice_t){0, 0, colors_empty()};

  return (choice_t){row, col, colors_rightmost(grid->cells[row][col])};
}

grid_t* grid_solver(grid_t* grid,
                    const solver_mode_t mode,
                    bool verbose,
                    int* counter,
                    FILE* fd) {
  if (!grid)
    return NULL;

  status_t status = grid_heuristics(grid);

  if (status == grid_inconsistent)
    return NULL;

  if (status == grid_solved) {
    if (mode == mode_all) {
      (*counter)++;
      fprintf(fd, "Solution %d:\n", *counter);
    } else {
      fprintf(fd, "Solution :\n\n");
    }

    grid_print(grid, fd);
    fprintf(fd, "\n");

    return (mode == mode_all) ? NULL : grid;
  }

  choice_t choice = grid_choice(grid);
  if (grid_choice_is_empty(choice))
    return NULL;

  if (verbose)
    grid_choice_print(choice, fd);

  colors_t remaining_colors = grid->cells[choice.row][choice.column];

  while (remaining_colors != colors_empty()) {
    colors_t color_to_try = colors_rightmost(remaining_colors);
    choice.color = color_to_try;

    if (verbose) {
      grid_choice_print(choice, fd);
    }

    grid_t* grid_cpy = grid_copy(grid);
    if (!grid_cpy)
      return NULL;

    grid_choice_apply(grid_cpy, choice);

    grid_t* solved_grid = grid_solver(grid_cpy, mode, verbose, counter, fd);

    if (solved_grid) {
      if (solved_grid != grid_cpy)
        grid_free(grid_cpy);

      return solved_grid;
    }

    grid_free(grid_cpy);
    grid_choice_discard(grid, choice);
    remaining_colors = colors_subtract(remaining_colors, color_to_try);
  }

  return NULL;
}
