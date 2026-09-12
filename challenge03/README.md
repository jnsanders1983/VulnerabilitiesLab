# Challenge 03 — Stack Corruption

## Objective

Understand how an out-of-bounds array access can corrupt another value stored on the stack.

Challenge 02 demonstrated that an out-of-bounds write can modify memory outside an array.

Challenge 03 takes the next step:

> An out-of-bounds write can modify another object stored in stack memory.

The goal is to understand the relationship between:

- C arrays
- Array indexes
- Pointer arithmetic
- Stack layout
- Out-of-bounds access
- Memory corruption
- Undefined behavior

This challenge is intentionally vulnerable and should only be run in the controlled local lab environment.

---

## Prerequisites

This challenge assumes familiarity with:

- Basic C syntax
- Variables and arrays
- Pointers
- Stack memory
- GDB
- Basic GDB commands such as:
  - `break`
  - `run`
  - `next`
  - `print`
  - `x`
  - `info frame`

Challenge 01 introduced stack frames, addresses, pointers, and saved return addresses.

Challenge 02 introduced out-of-bounds memory access.

---

## Directory Structure

The challenge is organized as:

    challenge03/
    ├── README.md
    └── challenge03.c

The compiled executable is placed in:

    bin/challenge03

---

# Source Code

The initial version of the program is:

    #include <stdio.h>

    int main(void)
    {
        int buffer[4];
        int target = 0x12345678;

        buffer[0] = 1;
        buffer[1] = 2;
        buffer[2] = 3;
        buffer[3] = 4;

        printf("buffer: %p\n", (void *)buffer);
        printf("target: 0x%x\n", target);

        return 0;
    }

The program contains:

    int buffer[4];

and:

    int target = 0x12345678;

The array has four valid elements:

    buffer[0]
    buffer[1]
    buffer[2]
    buffer[3]

Any other index is outside the bounds of the array.

---

# Build

From the repository root:

    make

The executable should be created at:

    bin/challenge03

Run it normally with:

    ./bin/challenge03

The initial program should display the buffer address and the value of `target`.

---

# Start GDB

The Makefile provides a convenient target:

    make debug03

Alternatively:

    gdb ./bin/challenge03

Set a breakpoint at `main`:

    break main

Start the program:

    run

Step through the declarations until the variables have been initialized:

    next
    next

---

# Examine the Array

First inspect the array:

    print buffer

The initial value should be:

    $1 = {0, 0, 0, 0}

The array contains four integers.

Examine the address of the array:

    print &buffer

The address has the type:

    int (*)[4]

This represents a pointer to the entire four-element array.

To examine individual elements:

    print &buffer[0]
    print &buffer[1]
    print &buffer[2]
    print &buffer[3]

Each `int` occupies four bytes, so the addresses increase by four bytes.

For example, the observed layout was:

    buffer[0] → 0x7fffffffd4c0
    buffer[1] → 0x7fffffffd4c4
    buffer[2] → 0x7fffffffd4c8
    buffer[3] → 0x7fffffffd4cc

Therefore, the next address would be:

    buffer[4] → 0x7fffffffd4d0

However, `buffer[4]` is not a valid element.

The valid indexes are only:

    0
    1
    2
    3

---

# Examine target

Now examine the address of `target`:

    print &target

The observed address was:

    0x7fffffffd4bc

Compare that with the beginning of the array:

    buffer[0] → 0x7fffffffd4c0
    target    → 0x7fffffffd4bc

The addresses are four bytes apart.

This means that, in this particular compiled program, the compiler placed `target` immediately before the array in memory.

The observed layout is:

    Higher addresses
            │
            ▼

    0x7fffffffd4cc    buffer[3]
    0x7fffffffd4c8    buffer[2]
    0x7fffffffd4c4    buffer[1]
    0x7fffffffd4c0    buffer[0]
    0x7fffffffd4bc    target

            │
            ▼
    Lower addresses

The important point is that the exact stack layout is determined by the compiler and build.

C does not guarantee that local variables will always be arranged this way.

---

# Verify the Distance

GDB can calculate the difference between the addresses:

    print &buffer[0] - &target

The observed result was:

    $8 = 1

The difference is one `int`.

This confirms that `target` is immediately before `buffer[0]` in the observed stack layout.

---

# Examine Raw Memory

Use GDB's memory examination command:

    x/6wx &target

The observed memory was:

    0x7fffffffd4bc: 0x12345678      0x00000000      0x00000000      0x00000000
    0x7fffffffd4cc: 0x00000000      0x00000000

The first value:

    0x12345678

is the value of `target`.

The following values correspond to surrounding stack memory.

Raw memory does not inherently contain C variable labels. GDB is interpreting addresses using information from the compiled program.

---

# Understanding Negative Array Indexes

C array indexing is based on pointer arithmetic.

Conceptually:

    buffer[index]

corresponds to accessing memory relative to the beginning of the array.

For the valid indexes:

    buffer[0]
    buffer[1]
    buffer[2]
    buffer[3]

the program accesses the four elements of the array.

A negative index moves in the opposite direction from the beginning of the array.

For example:

    buffer[-1]

refers to the `int` immediately before `buffer[0]` in memory.

This is not a valid element of the four-element array.

It is an out-of-bounds access.

---

# Demonstrate the Out-of-Bounds Read

We can demonstrate this without changing the source code.

Set an index in GDB:

    set $index = -1

Read the value:

    print buffer[$index]

The observed result was:

    $9 = 305419896

The decimal value:

    305419896

is:

    0x12345678

which is the value originally stored in `target`.

Now examine the address:

    print &buffer[$index]

The observed result was:

    $10 = (int *) 0x7fffffffd4bc

Compare that with:

    &target = 0x7fffffffd4bc

They are the same address.

GDB also confirmed this directly:

    print &buffer[-1]

Result:

    $11 = (int *) 0x7fffffffd4bc

Therefore, in this particular execution:

    buffer[-1]

and:

    target

refer to the same memory location.

---

# Important: Why Does This Work?

It works because of the particular memory layout produced by this build.

The array starts at:

    0x7fffffffd4c0

An `int` occupies four bytes.

Moving back one `int` gives:

    0x7fffffffd4bc

which happens to be where `target` is located.

Conceptually:

    buffer[0] → 0x7fffffffd4c0
    buffer[-1] → 0x7fffffffd4bc
                         │
                         ▼
                       target

This is an out-of-bounds access.

The fact that it reaches `target` is a consequence of the actual stack layout.

---

# Demonstrate the Out-of-Bounds Write

Before modifying memory, verify the value of `target`:

    print target

The original value is:

    305419896

which is:

    0x12345678

Now write through the invalid array index:

    set buffer[-1] = 0x41414141

Then inspect `target`:

    print target

The observed result was:

    $13 = 1094795585

The decimal value:

    1094795585

is:

    0x41414141

Confirm the actual memory contents:

    x/wx &target

The observed result was:

    0x7fffffffd4bc: 0x41414141

The memory that originally contained:

    0x12345678

now contains:

    0x41414141

The value of `target` has been corrupted.

---

# Why 0x41414141?

The value:

    0x41414141

is commonly used when examining memory corruption because hexadecimal `0x41` corresponds to the ASCII character:

    A

Therefore:

    0x41414141

represents four `A` bytes:

    A A A A

Using a recognizable pattern makes corrupted memory easier to identify when examining a debugger or memory dump.

---

# What We Just Demonstrated

The sequence was:

    target = 0x12345678

Then:

    buffer[-1] = 0x41414141

Because `buffer[-1]` happened to refer to the same address as `target`, the write changed the value of `target`.

Before:

    target
        │
        ▼
    0x7fffffffd4bc
        │
        ▼
    0x12345678

After:

    buffer[-1] = 0x41414141

        │
        ▼
    0x7fffffffd4bc
        │
        ▼
    0x41414141

This is stack memory corruption.

---

# Examine the Stack Frame

Use:

    info frame

The observed frame information included:

    Stack level 0, frame at 0x7fffffffd4f0:
     rip = 0x55555555518b in main (challenge03/challenge03.c:8);
     saved rip = 0x7ffff7c2a601
     Arglist at 0x7fffffffd4e0
     Locals at 0x7fffffffd4e0
     Saved registers:
      rbp at 0x7fffffffd4e0, rip at 0x7fffffffd4e8

The important point is that the local variables exist within the stack frame.

The stack frame also contains other information used by the function, including saved register state and the saved return address.

We are not corrupting the return address in this challenge.

That will be investigated in a later challenge.

---

# Stack Layout Is Not Guaranteed

It is important not to memorize the exact addresses from this experiment.

For example, this address:

    0x7fffffffd4c0

may be different the next time the program runs.

This is normal.

Modern Linux systems use mechanisms such as ASLR that can cause stack addresses to vary between executions.

More importantly, the compiler can also change the arrangement of local variables depending on:

- Compiler version
- Compiler options
- Optimization level
- Source-code changes
- Debugging options
- Architecture
- ABI

Therefore, the important lesson is not:

    target is always buffer[-1]

The correct lesson is:

> An out-of-bounds access can reach unintended memory, and the exact memory affected depends on the actual memory layout.

---

# Source-Level Vulnerability

After understanding the behavior in GDB, the vulnerability can be placed directly into the C source.

The vulnerable section is:

    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;
    buffer[3] = 4;

    buffer[-1] = 0x41414141;

The program can then print the corrupted value:

    printf("target after overflow: 0x%x\n", target);

A complete vulnerable version is:

    #include <stdio.h>

    int main(void)
    {
        int buffer[4];
        int target = 0x12345678;

        buffer[0] = 1;
        buffer[1] = 2;
        buffer[2] = 3;
        buffer[3] = 4;

        buffer[-1] = 0x41414141;

        printf("buffer: %p\n", (void *)buffer);
        printf("target: 0x%x\n", target);
        printf("target after overflow: 0x%x\n", target);

        return 0;
    }

Build it with:

    make

Then run:

    ./bin/challenge03

The output should show that `target` has been changed to:

    0x41414141

The exact addresses may differ between executions.

---

# Why This Is Undefined Behavior

The array was declared as:

    int buffer[4];

Therefore the valid indexes are:

    buffer[0]
    buffer[1]
    buffer[2]
    buffer[3]

This access:

    buffer[-1]

is outside the bounds of the array.

C does not provide automatic bounds checking for ordinary arrays.

The compiler therefore has no requirement to make this access safe.

The result is undefined behavior.

Undefined behavior does not necessarily mean that the program will immediately crash.

Possible results include:

- A neighboring value is changed
- Padding or unused memory is changed
- Another local variable is corrupted
- A pointer is corrupted
- Control-flow data is eventually corrupted
- The program crashes
- The program appears to work normally
- Different behavior occurs after a small code or compiler change

This is why memory corruption vulnerabilities can be difficult to reason about by looking only at whether a program crashes.

---

# Challenge 02 vs Challenge 03

Challenge 02 demonstrated an out-of-bounds write beyond the end of an array:

    buffer[4] = 0x41414141;

That write reached memory outside the array, but the specific memory did not correspond to `secret` in that stack layout.

Challenge 03 demonstrates an out-of-bounds write in the opposite direction:

    buffer[-1] = 0x41414141;

In this stack layout, that address happened to contain `target`.

The progression is:

    Challenge 02

    Array
      │
      └──► Out-of-bounds memory
                │
                └──► Unknown/other stack data


    Challenge 03

    Array
      │
      └──► Out-of-bounds memory
                │
                └──► target
                       │
                       └──► observable corruption

The important concept is that memory corruption becomes more meaningful when we can identify what data was affected.

---

# Key Concepts

## 1. Arrays Have Bounds

For:

    int buffer[4];

the valid indexes are:

    0 through 3

Indexes outside that range are invalid.

---

## 2. Array Indexing Uses Pointer Arithmetic

Array indexing is based on calculating an address relative to the beginning of the array.

For an `int` array, each element is four bytes in this environment.

Therefore:

    buffer[0]

is at the beginning of the array.

    buffer[1]

is four bytes later.

    buffer[-1]

is four bytes before the beginning.

---

## 3. Out-of-Bounds Access Is Undefined Behavior

C does not automatically prevent the program from accessing memory outside an array.

The CPU may still perform the memory access.

Whether that memory is:

- mapped
- readable
- writable
- useful
- dangerous

depends on the actual process memory layout.

---

## 4. Stack Corruption Does Not Automatically Mean Control-Flow Corruption

In this challenge, we corrupted:

    target

but the program's return address remained intact.

The corrupted value was data.

Later challenges will investigate what happens when memory corruption affects data that influences program execution.

---

## 5. Addresses Are Less Important Than Relationships

The exact address:

    0x7fffffffd4bc

is not the important part.

The important relationship is:

    target
       ↓
    4 bytes before
       ↓
    buffer[0]

Therefore:

    buffer[-1]

reached:

    target

on this build.

---

# Suggested GDB Experiment

Repeat the experiment from scratch to see whether the absolute addresses change.

Start GDB again:

    make debug03

Then:

    break main
    run

Step until the variables have been initialized:

    next
    next

Check:

    print &buffer[0]
    print &target

Then calculate:

    print &buffer[0] - &target

If the difference remains:

    1

the relative layout is the same.

The absolute addresses may still be different.

This demonstrates why exploit analysis should focus on memory relationships and offsets rather than assuming fixed addresses.

---

# Completion Checklist

You have completed Challenge 03 when you can explain:

- [x] Why `buffer[0]` through `buffer[3]` are valid.
- [x] Why `buffer[-1]` is out of bounds.
- [x] How GDB can show the address of each array element.
- [x] How to locate another variable on the stack.
- [x] How to compare the addresses of two objects.
- [x] How pointer arithmetic explains `buffer[-1]`.
- [x] How an out-of-bounds read can expose another value.
- [x] How an out-of-bounds write can corrupt another value.
- [x] Why `0x41414141` is useful as a recognizable corruption pattern.
- [x] Why the exact stack layout is compiler/build dependent.
- [x] Why absolute stack addresses can change between executions.
- [x] Why undefined behavior does not necessarily cause a crash.
- [x] Why data corruption is different from control-flow corruption.

---

# Takeaway

Challenge 02 showed that an array access can reach memory outside the array.

Challenge 03 demonstrated the consequence more clearly:

> An out-of-bounds write can corrupt another object stored in memory.

The important chain is:

    Invalid index
        ↓
    Invalid memory address
        ↓
    Memory write
        ↓
    Existing data is overwritten
        ↓
    Program state is corrupted

In this challenge, the corrupted data was simply an integer:

    target

The next step is to investigate what happens when corrupted memory affects something that controls **where the program executes**.

That will lead into **Challenge 04 — Controlled Control Flow**.