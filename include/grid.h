#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stdio.h>

#include <colors.h>

#define MAX_GRID_SIZE 64
#define EMPTY_CELL '_'

static const char color_table[] = "123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "@"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "&*";

/* Sudoku grid (forward declaration to hide the implementation) */
typedef struct _grid_t grid_t;

typedef enum { grid_solved, grid_unsolved, grid_inconsistent } status_t;

typedef enum { mode_first, mode_all } solver_mode_t;

typedef struct {
  size_t row;
  size_t column;
  colors_t color;
} choice_t;

/*
@brief Allocates memory for a sudoku grid of given `size`
@param size The size of the grid (number of rows/columns)
@return A pointer to the allocated grid_t structure, or NULL on failure
*/
grid_t* grid_alloc(size_t size);

/*
@brief Frees the memory allocated for a sudoku grid
@param grid A pointer to the grid_t structure to free
*/
void grid_free(grid_t* grid);

/*
@brief Prints the sudoku grid to the specified file descriptor `fd`
@param grid A pointer to the grid_t structure to print
@param fd The file descriptor to print the grid to
@return The number of characters printed
*/
size_t grid_print(const grid_t* grid, FILE* fd);

/*
@brief Checks if a character is valid in the sudoku grid of given size
@param grid A pointer to the grid_t structure to check
@param c The character to check
@return true if the character is valid, false otherwise
*/
bool grid_check_char(const grid_t* grid, const char c);

/*
@brief Checks if the given size is valid for a sudoku grid
@param size The size to check
@return true if the size is valid, false otherwise
*/
bool grid_check_size(const size_t size);

/*
@brief Creates a deep copy of the given sudoku grid
@param grid A pointer to the grid_t structure to copy
@return A pointer to the copied grid_t structure, or NULL on failure
*/
grid_t* grid_copy(const grid_t* grid);

/*
@brief Retrieves the string representation of the cell at the specified
coordinates
@param grid A pointer to the grid_t structure
@param row The row index of the cell
@param column The column index of the cell
@return A dynamically allocated string representing the cell's content, or NULL
on failure
*/
char* grid_get_cell(const grid_t* grid, const size_t row, const size_t column);

/*
@brief Sets the content of the cell at the specified row and column
@param grid A pointer to the grid_t structure
@param row The row index of the cell
@param column The column index of the cell
@param color The character representing the color to set in the cell
*/
void grid_set_cell(grid_t* grid,
                   const size_t row,
                   const size_t column,
                   const char color);

/*
@brief Retrieves the size of the sudoku grid
@param grid A pointer to the grid_t structure
@return The size of the grid, or 0 if grid is NULL
*/
size_t grid_get_size(const grid_t* grid);

/*
@brief Detects if a grid contains only singletons.
@param grid A pointer to the grid_t structure
@return True if the grid only contains singltons, false otherwise
*/
bool grid_is_solved(grid_t* grid);

/*
 @brief Checks whether a grid is consistent.

 Verifies if all of the rows, columns, and square blocks
 violate the rules defined by `subgrid_inconsistency`.

 @param grid Pointer to the grid_t structure to check. Must not be NULL.
 @return true if the grid is consistent; false if `grid` is NULL or any
 subgrid is inconsistent.
*/
bool grid_is_consistent(grid_t* grid);

/*
 @brief Applies heuristics to each subgrid until no further changes occur.

 Iterates over rows, columns, and square blocks, applying heuristics such as
 cross-hatching, lone numbers, naked subsets, and hidden subsets. Stops when a
 fix-point is reached.

 @param grid Pointer to the grid_t structure to operate on. Must not be NULL.
 @return The status of the grid after heuristics are applied (solved, partial,
 or invalid).
*/
status_t grid_heuristics(grid_t* grid);

/*
@brief Recursive backtracking solver for the sudoku grid.

 Attempts to solve the sudoku grid using a recursive backtracking algorithm.

 @param grid Pointer to the grid_t structure to solve. Must not be NULL.
 @param mode The solving mode (first solution or all solutions).
 @param counter Pointer to an integer counting the number of solutions found.
 @param fd File descriptor to output solutions when in 'all' mode.

 @return A pointer to the solved grid_t structure if a solution is found in
         'first' mode; NULL otherwise. In 'all' mode, returns NULL after
         printing all solutions.
*/
grid_t* grid_solver(grid_t* grid,
                    const solver_mode_t mode,
                    bool verbose,
                    int* counter,
                    FILE* fd);

#endif /* GRID_H */