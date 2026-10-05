#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_COLORS 64

/* 64 bits integer that represents a color set */
typedef uint64_t colors_t;

/*
@brief Initializes a full color set .
@param size The number of colors to include in the full set
@return A colors_t representing the full set of colors
*/
colors_t colors_full(const size_t size);

/*
@brief Initializes an empty color set where no colors are present
@return A colors_t representing the empty set of colors
*/
colors_t colors_empty(void);

/*
@brief Initializes a color set with a single color at `color_id` index
@param color_id The ID of the color to include in the set, must be less than
`MAX_COLORS`
@return A colors_t representing the color set with the specified color
*/
colors_t colors_set(const size_t color_id);

/*
@brief Adds a single color represented by `color_id` to the color set `colors`
@param colors The original color set
@param color_id The index of the color to add, must be less than `MAX_COLORS`
@return A colors_t representing the updated color set
*/
colors_t colors_add(const colors_t colors, const size_t color_id);

/*
@brief Removes a color from the color set
@param colors The original color set
@param color_id The index of the color to remove
@return A colors_t representing the updated color set
*/
colors_t colors_discard(const colors_t colors, const size_t color_id);

/*
@brief Checks if a color is in the color set
@param colors The color set to check
@param color_id The index of the color to check for in the set
@return true if the color is in the set, false otherwise
*/
bool colors_is_in(const colors_t colors, const size_t color_id);

/*
@brief Negates the color set
@param colors The original color set
@return A colors_t representing the negated color set
*/
colors_t colors_negate(const colors_t colors);

/*
@brief Computes the intersection of two color sets
@param colors1 The first color set
@param colors2 The second color set
@return A colors_t representing the intersection of the two color sets
*/
colors_t colors_and(const colors_t colors1, const colors_t colors2);

/*
@brief Computes the union of two color sets
@param colors1 The first color set
@param colors2 The second color set
@return A colors_t representing the union of the two color sets
*/
colors_t colors_or(const colors_t colors1, const colors_t colors2);

/*
@brief Computes the symmetric difference of two color sets
@param colors1 The first color set
@param colors2 The second color set
@return A colors_t representing the symmetric difference of the two color sets
*/
colors_t colors_xor(const colors_t colors1, const colors_t colors2);

/*
@brief Computes the difference of two color sets
@param colors1 The first color set
@param colors2 The second color set
@return A colors_t representing the difference of the two color sets
*/
colors_t colors_subtract(const colors_t colors1, const colors_t colors2);

/*
@brief Checks if two color sets are equal
@param colors1 The first color set
@param colors2 The second color set
@return true if the two color sets are equal, false otherwise
*/
bool colors_is_equal(const colors_t colors1, const colors_t colors2);

/*
@brief Checks if the first color set is a subset of the second color set
@param colors1 The first color set
@param colors2 The second color set
@return true if the first color set is a subset of the second, false otherwise
*/
bool colors_is_subset(const colors_t colors1, const colors_t colors2);

/*
@brief Checks if the color set is a singleton (contains exactly one color)
@param colors The color set to check
@return true if the color set is a singleton, false otherwise
*/
bool colors_is_singleton(const colors_t colors);

/*
@brief Counts the number of colors in the color set.
@param colors The color set to count
@return The number of colors in the set
*/
size_t colors_count(const colors_t colors);

/*
@brief Finds the rightmost color in the color set
@param colors The color set to search
@return The ID of the rightmost color, or -1 if the set is empty
*/
colors_t colors_rightmost(const colors_t colors);

/*
@brief Finds the leftmost color in the color set.
@param colors The color set to search
@return The ID of the leftmost color, or -1 if the set is empty
*/
colors_t colors_leftmost(const colors_t colors);

/*
@brief Finds a random color in the color set
@param colors The color set to search
@return The ID of a random color, or -1 if the set is empty
*/
colors_t colors_random(const colors_t colors);

/*
 @brief Checks whether a subgrid is inconsistent.

 A subgrid is considered inconsistent if it has empty cells,
 duplicate singletons, or missing required colors.

 @param subgrid Array of colors_t* representing the subgrid cells.
 @param size The number of cells in the subgrid.
 @return true if the subgrid is inconsistent; false otherwise.
*/
bool subgrid_inconsistency(colors_t* subgrid[], const size_t size);

/*
 @brief Executes a set of heuristics on a single subgrid.

 Applies cross-hatching, lone numbers, naked subsets,
 and hidden subsets to the cells of the given subgrid. Each heuristic is
 applied **once**, and the function returns whether any changes were made.

 @param subgrid Array of pointers to colors_t representing the subgrid cells.
 @param size The number of cells in the subgrid (e.g., row/column/block length).
 @return true if any changes were made to the subgrid; false if no changes
 occurred.
*/
bool subgrid_heuristics(colors_t* subgrid[], const size_t size);

#endif /* COLORS_H */