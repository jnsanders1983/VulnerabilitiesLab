# VulnerabilitiesLab

A hands-on C security lab for learning how memory corruption vulnerabilities work from the ground up.

The project contains intentionally vulnerable C programs that can be analyzed and exploited in a controlled local environment. Each challenge focuses on understanding what happens in memory, how a vulnerability affects program behavior, and how the vulnerability can ultimately be mitigated.

## Current Challenge

**Challenge 01 — Stack & Pointers**

Introduces:

* Stack memory
* Local variables
* Pointers and addresses
* Stack frames
* Inspecting memory with GDB

This challenge does **not** contain an exploitable vulnerability yet.

## Requirements

* GCC or Clang
* GDB
* Git
* Linux, WSL, or another Unix-like environment

## Clone

```bash
git clone https://github.com/YOUR_USERNAME/vulnlab.git
cd vulnlab
```

## Build

```bash
gcc -g -O0 -Wall -Wextra challenge01.c -o challenge01
```

## Run

```bash
./challenge01
```

## Debug with GDB

```bash
gdb ./challenge01
```

Inside GDB:

```text
break main
run
```

From there, inspect variables, pointers, memory, and stack frames as described in the challenge.

## Purpose

This project is an educational security lab. All vulnerabilities are intentionally created for experimentation and should be run only in an isolated environment you control.

