# High-Performance Sudoku Engine & Heuristic Solver

A high-performance Sudoku solving engine written in C11. It combines bitwise candidate representations, constraint propagation heuristics, and recursive backtracking guided by the **Minimum Remaining Values (MRV)** heuristic to solve arbitrary grids from $1\times 1$ up to $64\times 64$.

---

## Key Features

- **Scalable Grid Dimensions**: Solves Sudoku grids of size $N \times N$ where $N \in \{1, 4, 9, 16, 25, 36, 49, 64\}$.
- **64-bit Bitset Representation (`colors_t`)**:
  - Each cell's candidate set is encoded into a single `uint64_t`.
  - Set operations (union, intersection, difference, subset testing) are evaluated in single-cycle bitwise instructions (`|`, `&`, `^`, `~`).
  - Bit counting utilizes a parallel **SWAR (SIMD Within A Register)** algorithm for instant popcount.
- **Deductive Constraint Propagation Heuristics**:
  - **Cross-Hatching**: Eliminates confirmed singletons from peer cells within the same row, column, and subgrid block.
  - **Lone Numbers (Hidden Singles)**: Detects candidates appearing exactly once across a row, column, or block and assigns them immediately.
  - **Naked Subsets**: Identifies naked pairs, triples, and $k$-tuples whose candidates can be eliminated from other cells in the subgrid.
  - **Hidden Subsets**: Identifies candidate groups confined to $k$ cells and strips extraneous candidates.
- **Optimized Backtracking Solver**:
  - **Minimum Remaining Values (MRV / Fail-First)**: Selects the cell with the fewest remaining candidate colors to minimize the search tree branching factor.
  - Early-exit optimization when binary branches (2 candidates) are discovered.
- **Multiple Solving Modes**:
  - First-solution search (default fast solver).
  - Exhaustive search (`-a / --all`) to count and enumerate all possible valid solutions.
  - Verbose decision tracing (`-v / --verbose`).
  - Grid generator mode (`-g / --generate`).

---

## Project Structure

```
├── include/
│   ├── colors.h     # Bitset candidate type and set operations (SWAR, bit manipulation)
│   └── grid.h       # Grid data structures, heuristic prototypes, and solver API
├── src/
│   ├── colors.c     # Bitset operations, heuristic subgrid algorithms
│   ├── grid.c       # Grid memory management, constraint propagation, MRV backtracking
│   ├── sudoku.c     # CLI argument parsing, file parser, and application entry point
│   ├── sudoku.h     # Version declarations
│   └── Makefile     # Source compilation rules
├── tests/
│   ├── grid_9x9_easy.txt   # Sample 9x9 test grid
│   └── grid_9x9_hard.txt   # Hard 9x9 test grid
├── Makefile         # Top-level build automation
└── .gitignore       # Build artifacts and editor file exclusions
```

---

## Getting Started

### Prerequisites

- GCC or Clang supporting C11 (e.g. `gcc >= 7.0`)
- GNU Make

### Building the Project

Compile the executable:
```bash
make
```

To clean compiled objects and binaries:
```bash
make clean
```

---

## Usage

### Solving a Sudoku Grid

```bash
# Solve a grid (finds the first valid solution)
./sudoku tests/grid_9x9_easy.txt

# Enumerate all possible solutions and display solution count
./sudoku -a tests/grid_9x9_hard.txt

# Write solution output to a file
./sudoku -o solution.txt tests/grid_9x9_easy.txt

# Verbose mode: inspect branching decisions
./sudoku -v tests/grid_9x9_hard.txt
```

### Grid File Format

Grids are specified in plain text. Numbers, letters, and symbols are mapped to candidate colors (e.g. `1`-`9` for 9x9). Empty cells are represented by `_`:

```text
_ _ 3 _ 2 _ 6 _ _
9 _ _ 3 _ 5 _ _ 1
_ _ 1 8 _ 6 4 _ _
_ _ 8 1 _ 2 9 _ _
7 _ _ _ _ _ _ _ 8
_ _ 6 7 _ 8 2 _ _
_ _ 2 6 _ 9 5 _ _
8 _ _ 2 _ 3 _ _ 9
_ _ 5 _ 1 _ 3 _ _
```

Comments can be included using `#`:
```text
# Sample 9x9 grid
1 2 3 _ _ _ _ _ _
...
```

---

## License

This project is licensed under the MIT License.
