# Datasets 

This folder contains the example datasets used by the basic register allocation tests.
Each dataset includes a live range file, a register/settings file, and the expected output produced by the assignment tool.

## Files and folders

- `ranges/`
  - Contains live range input files for each test case.
  - Each file defines program points for variables, including begin/end markers used to build webs.

- `registers/`
  - Contains register settings files.
  - Each file specifies the number of registers and the allocation algorithm parameters used by the tool.

- `output/`
  - Contains example allocation outputs generated for the corresponding input pairs.
  - These files show how the implemented allocator assigns registers or spills webs to memory.

## Dataset table

| Range file     | Registers file   | Expected result  | Output file       |
| -------------- | ---------------- | ---------------- | ----------------- |
| `ranges1.txt`  | `registers2.txt` | 2 registers      | `allocation1.txt` |
| `ranges2.txt`  | `registers2.txt` | 2 registers      | `allocation2.txt` |
| `ranges3.txt`  | `registers2.txt` | 2 registers      | `allocation3.txt` |
| `ranges4.txt`  | `registers1.txt` | 1 register       | `allocation4.txt` |
| `ranges5.txt`  | `registers1.txt` | 1 register       | `allocation5.txt` |
| `ranges6.txt`  | `registers3.txt` | 3 registers      | `allocation6.txt` |

## Notes

- `rangesX.txt` files describe the live ranges that the parser converts into webs.
- `registersX.txt` files control how many physical registers are available and which algorithm variant is used.
- `allocationX.txt` files are example outputs; the exact web numbering or register labels may differ if the allocator uses a different valid assignment.
- The datasets are intended to exercise the basic allocation behavior and validate the tool on small representative inputs.
