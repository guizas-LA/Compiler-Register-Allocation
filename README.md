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

The T1.1 assignment tool is implemented with:

- interactive command-line menu;
- batch execution;
- input validation;
- parsed input data structures;
- interference graph based on the provided `Graph` class;
- output file generation;
- detailed result display;
- basic graph-coloring allocation support.

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
