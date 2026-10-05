#define _DEFAULT_SOURCE

#include "colors.h"

#include <unistd.h>

#include <time.h>

colors_t colors_full(const size_t size) {
  if (size >= MAX_COLORS)
    return UINT64_MAX;
  return ((colors_t)1 << size) - 1;
}

colors_t colors_empty(void) {
  return 0;
}

colors_t colors_set(const size_t color_id) {
  if (color_id >= MAX_COLORS)
    return 0;
  return 1ULL << color_id;
}

colors_t colors_add(const colors_t colors, const size_t color_id) {
  return colors | colors_set(color_id);
}

colors_t colors_discard(const colors_t colors, const size_t color_id) {
  return colors & ~colors_set(color_id);
}

bool colors_is_in(const colors_t colors, const size_t color_id) {
  return colors & colors_set(color_id);
}

colors_t colors_negate(const colors_t colors) {
  return ~colors;
}

colors_t colors_and(const colors_t colors1, const colors_t colors2) {
  return colors1 & colors2;
}

colors_t colors_or(const colors_t colors1, const colors_t colors2) {
  return colors1 | colors2;
}

colors_t colors_xor(const colors_t colors1, const colors_t colors2) {
  return colors1 ^ colors2;
}

colors_t colors_subtract(const colors_t colors1, const colors_t colors2) {
  return colors1 & ~colors2;
}

bool colors_is_equal(const colors_t colors1, const colors_t colors2) {
  return colors1 == colors2;
}

bool colors_is_subset(const colors_t colors1, const colors_t colors2) {
  return (colors1 & colors2) == colors1;
}

bool colors_is_singleton(const colors_t colors) {
  return colors && !(colors & (colors - 1));
}

size_t colors_count(const colors_t colors) {
  /* 64-bit parallel bit-counting (SWAR algorithm) */
  colors_t cpy = colors;
  cpy = cpy - ((cpy >> 1) & 0x5555555555555555ULL);
  cpy = (cpy & 0x3333333333333333ULL) + ((cpy >> 2) & 0x3333333333333333ULL);
  cpy = (cpy + (cpy >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
  return (cpy * 0x0101010101010101ULL) >> 56;
}

colors_t colors_rightmost(const colors_t colors) {
  return colors & -colors;
}

colors_t colors_leftmost(const colors_t colors) {
  if (colors == 0)
    return 0;

  /* Binary search for most significant bit (MSB) */
  colors_t colors_temp = colors;
  colors_t pos = 0;

  if (colors_temp >> 32) {
    colors_temp >>= 32;
    pos += 32;
  }

  if (colors_temp >> 16) {
    colors_temp >>= 16;
    pos += 16;
  }

  if (colors_temp >> 8) {
    colors_temp >>= 8;
    pos += 8;
  }

  if (colors_temp >> 4) {
    colors_temp >>= 4;
    pos += 4;
  }

  if (colors_temp >> 2) {
    colors_temp >>= 2;
    pos += 2;
  }

  if (colors_temp >> 1)
    pos += 1;

  return 1ULL << pos;
}

static int get_random(void) {
  static bool seed_initialized = false;
  if (!seed_initialized) {
    srandom(time(NULL) * getpid());
    seed_initialized = true;
  }

  return random();
}

colors_t colors_random(const colors_t colors) {
  if (colors == 0)
    return colors_empty();

  colors_t colors_copy = colors;
  size_t index = get_random() % colors_count(colors);

  for (size_t i = 0; i < index; i++) {
    colors_copy -= colors_rightmost(colors_copy);
  }

  return colors_rightmost(colors_copy);
}

bool subgrid_inconsistency(colors_t* subgrid[], const size_t size) {
  if (!subgrid)
    return false;

  colors_t seen_singletons = colors_empty();
  colors_t colors_union = colors_empty();

  for (size_t i = 0; i < size; i++) {
    if (*subgrid[i] == colors_empty())
      return true;

    if (colors_is_singleton(*subgrid[i])) {
      if (!colors_is_equal(colors_and(*subgrid[i], seen_singletons),
                           colors_empty()))
        return true;

      seen_singletons = colors_or(seen_singletons, *subgrid[i]);
    }

    colors_union = colors_or(colors_union, *subgrid[i]);
  }

  return !colors_is_equal(colors_union, colors_full(size));
}

/*
 @brief Applies the cross-hatching heuristic to a subgrid.
 Eliminates singleton colors from all multi-candidate cells within the subgrid.
 @param subgrid Array of pointers to colors_t representing the subgrid cells.
 @param size Number of cells in the subgrid.
 @return true if any cell was modified; false otherwise.
*/
static bool subgrid_cross_hatching(colors_t* subgrid[], const size_t size) {
  colors_t singletons_union = colors_empty();

  for (size_t i = 0; i < size; i++) {
    colors_t cell = *subgrid[i];

    if (colors_is_singleton(cell))
      singletons_union |= cell;
  }

  if (singletons_union == colors_empty())
    return false;

  bool subgrid_changed = false;

  for (size_t i = 0; i < size; i++) {
    colors_t cell = *subgrid[i];

    if (!colors_is_singleton(cell)) {
      colors_t new_colors = colors_subtract(cell, singletons_union);

      if (new_colors != cell) {
        *subgrid[i] = new_colors;
        subgrid_changed = true;
      }
    }
  }

  return subgrid_changed;
}

/*
 @brief Applies the lone number heuristic to a subgrid.
 For each color that appears in exactly one cell of the subgrid, fixes that cell
 to contain only that color.
 @param subgrid Array of pointers to colors_t representing the subgrid cells.
 @param size Number of cells in the subgrid.
 @return true if any cell was modified; false otherwise.
*/
static bool subgrid_lone_number(colors_t* subgrid[], const size_t size) {
  colors_t present_once = colors_empty();
  colors_t present_twice = colors_empty();

  for (size_t i = 0; i < size; i++) {
    colors_t current_mask = *subgrid[i];
    present_twice |= present_once & current_mask;
    present_once |= current_mask;
  }

  present_once ^= present_twice;

  if (present_once == colors_empty())
    return false;

  bool subgrid_changed = false;

  for (size_t i = 0; i < size; i++) {
    colors_t cell = *subgrid[i];
    colors_t intersection = cell & present_once;

    if (intersection != colors_empty() && intersection != cell) {
      *subgrid[i] = intersection;
      subgrid_changed = true;
    }
  }

  return subgrid_changed;
}

/*
 @brief Applies the naked subset heuristic to a subgrid.
 Identifies sets of cells where the number of candidate colors equals the number
 of cells containing them. Eliminates these candidates from all other cells in
 the subgrid.
 @param subgrid Array of pointers to colors_t representing the subgrid cells.
 @param size Number of cells in the subgrid.
 @return true if any cell was modified; false otherwise.
*/
static bool subgrid_naked_subset(colors_t* subgrid[], const size_t size) {
  bool subgrid_changed = false;
  colors_t unique_masks[size];
  size_t mask_count[size];
  size_t unique_count = 0;

  /* build mask table, skip singletons */
  for (size_t i = 0; i < size; i++) {
    colors_t current_mask = *subgrid[i];

    if (colors_is_singleton(current_mask))
      continue;

    size_t j;
    for (j = 0; j < unique_count; j++) {
      if (current_mask == unique_masks[j]) {
        mask_count[j]++;
        break;
      }
    }

    if (j == unique_count) {
      unique_masks[unique_count] = current_mask;
      mask_count[unique_count] = 1;
      unique_count++;
    }
  }

  for (size_t i = 0; i < unique_count; i++) {
    colors_t current_mask = unique_masks[i];
    size_t mask_size = colors_count(current_mask);

    if (mask_size != mask_count[i] || mask_size < 2)
      continue;

    for (size_t j = 0; j < size; j++) {
      colors_t cell = *subgrid[j];

      if (colors_is_singleton(cell) || cell == current_mask)
        continue;

      colors_t new_colors = colors_subtract(cell, current_mask);

      if (new_colors != cell) {
        *subgrid[j] = new_colors;
        subgrid_changed = true;
      }
    }
  }

  return subgrid_changed;
}

/*
 @brief Applies the hidden subset heuristic to a subgrid.
 For all pairs of colors that appear together in exactly two cells, restricts
 those cells to only those colors, removing other candidates.
 @param subgrid Array of pointers to colors_t representing the subgrid cells.
 @param size Number of cells in the subgrid.
 @return true if any cell was modified; false otherwise.
*/
static bool subgrid_hidden_subset(colors_t* subgrid[], const size_t size) {
  bool subgrid_changed = false;

  for (size_t i = 0; i < size; i++) {
    colors_t cell_colors = *subgrid[i];
    size_t count = colors_count(cell_colors);

    if (count <= 1 || count > size)
      continue;

    size_t shared_count = 0;
    for (size_t j = 0; j < size; j++) {
      if (*subgrid[j] & cell_colors)
        shared_count++;
    }

    if (shared_count != count)
      continue;

    for (size_t j = 0; j < size; j++) {
      colors_t cell = *subgrid[j];

      if (cell & cell_colors) {
        colors_t restricted = cell & cell_colors;

        if (restricted != cell) {
          *subgrid[j] = restricted;
          subgrid_changed = true;
        }
      }
    }
  }

  return subgrid_changed;
}

bool subgrid_heuristics(colors_t* subgrid[], const size_t size) {
  bool subgrid_changed = subgrid_cross_hatching(subgrid, size);
  subgrid_changed |= subgrid_lone_number(subgrid, size);
  subgrid_changed |= subgrid_naked_subset(subgrid, size);
  subgrid_changed |= subgrid_hidden_subset(subgrid, size);

  return subgrid_changed;
}
