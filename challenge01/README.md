# Challenge 01 — Stack & Pointers

## Objective

This challenge introduces the basic relationship between C variables, memory addresses, pointers, and stack frames.

There is **no vulnerability in this challenge**.

The goal is to become comfortable using GDB to observe what the C program is doing in memory before introducing memory-corruption vulnerabilities.

By the end of this challenge, you should understand:

* Local variables
* Stack memory
* Memory addresses
* Pointers
* Pointer dereferencing
* Stack frames
* The instruction pointer (`RIP`)
* Saved return addresses
* How GDB maps machine-code addresses back to C source lines

## Source Code

`challenge01.c`:

```c
#include <stdio.h>

void inspect(int value)
{
    int local = 42;
    int *ptr = &local;

    printf("value: %d\n", value);
    printf("local: %d\n", local);
    printf("ptr:   %p\n", (void *)ptr);
}

int main(void)
{
    int number = 1234;

    printf("number: %d\n", number);
    printf("&number: %p\n", (void *)&number);

    inspect(number);

    return 0;
}
```

## Build

From the root of `VulnerabilitiesLab`:

```bash
make
```

The executable will be created at:

```text
bin/challenge01
```

Run it with:

```bash
./bin/challenge01
```

## GDB Investigation

Start GDB:

```bash
gdb ./bin/challenge01
```

Set a breakpoint at `main`:

```gdb
break main
```

Start the program:

```gdb
run
```

---

## 1. Local Variables

The program contains:

```c
int number = 1234;
```

When GDB stops on this line, the initialization may not have executed yet.

Check the value:

```gdb
print number
```

Then:

```gdb
next
```

GDB moves to the next source location.

Check again:

```gdb
print number
```

The value should now be:

```text
1234
```

### Important GDB concept

When GDB displays a source line, it is showing the source line associated with the current machine instruction.

Being stopped on a line does **not** necessarily mean that the source statement has already completed.

`next` executes the current source-level statement and stops at the next source location without stepping into called functions.

---

## 2. Inspecting an Address

Examine the address of `number`:

```gdb
print &number
```

This gives the memory address where `number` is stored.

The distinction is:

```text
number   → value stored in the variable
&number  → address where the variable is stored
```

---

## 3. Examining Memory Directly

Use:

```gdb
x/wd &number
```

The command means:

```text
x   examine memory
w   examine one word (4 bytes)
d   display as decimal
```

You should see the value of `number`.

You can also display the same memory as hexadecimal:

```gdb
x/wx &number
```

For example:

```text
0x7fffffffd...:  0x000004d2
```

`1234` decimal is `0x4d2` hexadecimal.

---

## 4. Entering `inspect()`

At:

```c
inspect(number);
```

use:

```gdb
step
```

`step` enters the called function.

You should now be inside:

```c
void inspect(int value)
```

and initially stopped at:

```c
int local = 42;
```

---

## 5. Observe Initialization

Before executing:

```c
int local = 42;
```

run:

```gdb
print local
```

On this system, GDB may show:

```text
$... = 0
```

The important point is that the source initialization has **not executed yet**.

Now:

```gdb
next
```

GDB moves to:

```c
int *ptr = &local;
```

Now:

```gdb
print local
```

should show:

```text
42
```

The initialization has now executed.

---

## 6. Understanding Pointers

At this point:

```c
int *ptr = &local;
```

has not yet executed.

Check:

```gdb
print ptr
```

The pointer has not been initialized by line 6 yet.

Now execute line 6:

```gdb
next
```

GDB should move to the first `printf()`.

Now:

```gdb
print ptr
```

The pointer should contain the address of `local`.

Check:

```gdb
print &local
```

and:

```gdb
print ptr
```

They should contain the same address.

For example:

```text
&local = 0x7fffffffd5cc
ptr    = 0x7fffffffd5cc
```

Now dereference the pointer:

```gdb
print *ptr
```

The result should be:

```text
42
```

Therefore:

```text
local   → 42

&local  → address of local

ptr     → address stored in ptr
           ↓
        address of local

*ptr    → value at that address
           ↓
           42
```

---

## 7. Examine the Memory Around `local`

Use:

```gdb
x/wd &local
```

This displays `local` as a decimal integer.

Then:

```gdb
x/wx &local
```

This displays the same memory as hexadecimal.

For example:

```text
0x7fffffffd5cc: 0x0000002a
```

`0x2a` hexadecimal is `42` decimal.

Now examine several words surrounding `local`:

```gdb
x/8wx &local
```

This displays 8 four-byte words beginning at the address of `local`.

The output will contain several values that we cannot immediately identify.

**Do not assume that every value has a particular meaning just because it appears near `local`.**

Raw memory has no labels. The debugger's symbol information and the program's machine instructions allow us to interpret it.

---

## 8. Inspect the Stack Frame

Run:

```gdb
info frame
```

This shows information about the current stack frame.

Important fields include:

* Stack level
* Frame address
* Current `RIP`
* Saved `RIP`
* Arguments
* Locals
* Saved registers

For example:

```text
Stack level 0, frame at 0x7fffffffd5f0:
 rip = ... in inspect (...:8);
 saved rip = 0x55555555525a
 ...
 Arglist at 0x7fffffffd5e0, args: value=1234
 Locals at 0x7fffffffd5e0
 ...
 Saved registers:
   rbp at 0x7fffffffd5e0, rip at 0x7fffffffd5e8
```

The exact addresses will vary.

---

## 9. Understanding `RIP`

On x86-64 systems, `RIP` is the **instruction pointer**.

It identifies the address of the instruction where the CPU is currently executing.

The stack frame also contains a **saved RIP**.

The saved RIP is the address where execution should continue after the current function returns.

These are different concepts:

```text
Current RIP
    ↓
Where the CPU is executing now

Saved RIP
    ↓
Where execution will resume after the function returns
```

---

## 10. Verify the Saved Return Address

Suppose `info frame` reports:

```text
saved rip = 0x55555555525a
```

Examine that address directly:

```gdb
x/gx 0x7fffffffd5e8
```

You should see the saved address stored in memory.

For example:

```text
0x7fffffffd5e8: 0x000055555555525a
```

The exact addresses will depend on your executable.

---

## 11. Map a Machine Address Back to C

A hexadecimal address such as:

```text
0x55555555525a
```

does not by itself tell you which C line it represents.

Ask GDB:

```gdb
info line *0x55555555525a
```

GDB can use the debugging information produced by:

```bash
-g
```

to map the machine-code address back to a source line.

For this challenge, the saved return address should correspond to:

```c
return 0;
```

in `main`.

You can also inspect the assembly and its relationship to the source with:

```gdb
disassemble /m main
```

And inspect the instruction itself with:

```gdb
x/i 0x55555555525a
```

For example, GDB may report:

```text
0x55555555525a <main+96>: mov $0x0,%eax
```

`<main+96>` means the instruction is 96 bytes into the compiled machine code for `main`.

It does **not** mean C line 96.

---

## 12. Finish the Function

Once the investigation is complete:

```gdb
finish
```

`finish` executes until the current function returns and GDB stops back in the calling function.

Then:

```gdb
continue
```

allows the program to finish normally.

---

## Key Concepts

### Variable

```c
int local = 42;
```

A variable has a value and occupies memory.

### Address

```c
&local
```

The `&` operator obtains the address where `local` is stored.

### Pointer

```c
int *ptr = &local;
```

`ptr` is a variable whose value is an address.

### Dereference

```c
*ptr
```

Dereferencing follows the address stored in `ptr` and accesses the value at that location.

### Stack Frame

When `inspect()` executes, it has its own stack frame containing information associated with the function, including local state and saved control-flow information.

### Saved Return Address

The saved RIP tells the processor where execution should resume when the current function returns.

---

## Challenge 01 Takeaway

The most important relationship from this challenge is:

```text
                 MEMORY
                   │
                   ▼
              address of local
                   │
                   │ contains
                   ▼
                  42
                   ▲
                   │
                 local
                   │
                   │
             &local = address
                   │
                   ▼
                  ptr
                   │
                   │ dereference
                   ▼
                 *ptr
                   │
                   ▼
                  42
```

We have not corrupted or manipulated any memory in this challenge.

We have simply learned how to **observe the normal state of a C program**.

That provides the foundation for the next challenge, where we will intentionally introduce an out-of-bounds memory access and observe what changes.
