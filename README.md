# VulnerabilitiesLab

A hands-on C security lab for learning how memory corruption vulnerabilities work from the ground up.

VulnerabilitiesLab is a progressive collection of small, intentionally vulnerable C programs designed to build a practical understanding of memory corruption, debugging, exploitation, and defense.

Each challenge builds on concepts introduced in previous challenges, progressing from understanding normal memory behavior to analyzing vulnerabilities, exploiting them in a controlled local environment, and applying mitigations.

## Learning Approach

```text
Understand → Observe → Exploit → Defend → Verify
```

The focus is on understanding **why** a vulnerability works, not simply following an exploit recipe.

## Environment

* C
* GCC or Clang
* GDB
* Make
* Linux / WSL

The lab is entirely local and does not require a network service or external target.

## Challenges

### Challenge 01 — Stack & Pointers

Introduces the fundamentals of stack memory, pointers, addresses, and stack frames.

See [`challenge01/README.md`](challenge01/README.md).

### Challenge 02 — Out-of-Bounds Memory Access

Introduces the first memory corruption vulnerability and examines its effects.

See [`challenge02/README.md`](challenge02/README.md).

*Additional challenges will be added progressively.*

## Build

From the repository root:

```bash
make
```

Executables are placed in:

```text
bin/
```

For example:

```bash
./bin/challenge01
```

## Debug

```bash
make debug
```

Or directly:

```bash
gdb ./bin/challenge01
```

## Safety

This repository contains intentionally vulnerable programs for educational purposes.

Run the challenges only in a controlled environment that you own or are authorized to use.
