# DA-Proj2

Compiler register allocation assignment tool for DA Project 2.

## Build

```bash
make
```

This creates the executable:

```bash
./myProg
```

## Documentation

The code uses Doxygen comments, including time-complexity notes for the most
relevant parsing, web-building, interference-graph and coloring routines.

Generate HTML documentation with:

```bash
make docs
```

The generated files are written to `docs/html/`.

## Interactive Mode

Run the tool without arguments:

```bash
./myProg
```

The menu lets you configure:

- ranges input file;
- registers/settings input file;
- output file;
- input loading and validation;
- allocation execution;
- result display and saving.

## Batch Mode

The project statement requires this interface:

```bash
./myProg -b ranges.txt registers.txt allocation.txt
```

Example:

```bash
./myProg -b basic/ranges/ranges1.txt basic/registers/registers2.txt basic/output/allocation1.txt
```

Errors and warnings are written to the console through `stderr`.

## Current Scope

The assignment tool is implemented with:

- interactive command-line menu;
- batch execution;
- input validation;
- parsed input data structures;
- interference graph based on the provided `Graph` class;
- output file generation;
- detailed result display;
- basic graph-coloring allocation support.
- web spilling allocation support.

## Implemented Algorithms

### Basic

`algorithm: basic`

Time complexity:
- build webs: O(W² · P)
- build interference graph: O(V · (V + E))
- single color attempt: O(V · R)
- basic allocator: O(V · R²)

The basic allocator tries to color the interference graph with the minimum number
of registers from `1` up to the configured limit. For each number of registers it
uses the simplification/stack strategy described in the project statement:

- repeatedly remove a web whose active degree is lower than the number of
  available colors;
- push removed webs onto a stack;
- if no such web exists, the graph is not colorable by the basic algorithm with
  that register count;
- pop the stack and assign the first register not used by already-colored
  neighbors.

If the graph cannot be colored with the provided number of registers, the output
uses `registers: 0` and assigns every web to memory with `M`.

### Spilling

`algorithm: spilling, K`

Time complexity: O(K · V · (V + E))

The spilling allocator first runs the basic allocator. If that fails, it tries to
spill as few webs as possible, from `1` up to `K`.

The spill heuristic sorts candidate webs by:

1. highest interference degree first;
2. longest live range as a tie-breaker;
3. lowest web id as a final deterministic tie-breaker.

Rationale: a high-degree web blocks many neighboring webs from sharing registers,
so spilling it usually removes more constraints from the interference graph.

### Splitting

`algorithm: splitting, K`

Time complexity: O(K · V · (V + E))

The splitting allocator first runs the basic allocator. If that fails, it tries
to split as few webs as possible, from `1` up to `K`.

The split heuristic uses the same priority as spilling:

1. highest interference degree first;
2. longest live range as a tie-breaker;
3. lowest web id as a deterministic tie-breaker.

Each selected web is split into two ordered sections around the midpoint of its
program points. The first derived web is marked as ending at the split boundary,
and the second is marked as beginning there. The interference graph is rebuilt
for the derived webs and the basic coloring algorithm is retried.

Rationale: high-degree, long webs are the best candidates because splitting them
can distribute their interferences across smaller derived webs, often turning a
dense graph into a colorable one without sending values to memory.

### Free

`algorithm: free`

Time complexity: O(V² + V · E)

The free allocator uses a DSATUR-based search:

- at each step it selects the uncolored web with the highest saturation degree,
  meaning the highest number of distinct colors already used by its neighbors;
- ties are broken by higher interference degree and then lower web id;
- it uses backtracking, so it can recover from an unlucky color choice;
- it tries to use the minimum number of registers from `1` up to the configured
  register limit.

If DSATUR cannot color the graph with the configured number of registers, the
free allocator applies a fallback spilling strategy. It tries to spill as few
webs as possible, using the same high-degree/long-live-range priority as the
spilling algorithm, and runs DSATUR again on the reduced graph.

Rationale: DSATUR tends to color constrained graphs more effectively than a
plain greedy order because it reacts dynamically to the colors already assigned
around each web. The fallback keeps the result valid even when the graph cannot
be colored within the available registers.

## Project Structure
```text
proj/
├── include/                  # Public headers
├── Makefile                  # Build commands
├── Doxyfile                  # Doxygen configuration
├── README.md                 # Project overview
├── src/                      # Source files
├── basic/                    # Basic algorithm datasets
├── data_structures/          # Provided graph/auxiliary structures
└── .gitignore                # Git ignore rules
```
