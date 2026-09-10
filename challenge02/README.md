# Challenge 02 — Out-of-Bounds Memory Access

## Objective

This challenge introduces the first intentionally vulnerable program in the VulnerabilitiesLab project.

The goal is to understand:

- How C arrays are stored in memory
- How array indexes correspond to memory addresses
- What happens when an array is accessed outside its valid bounds
- The difference between an out-of-bounds read and an out-of-bounds write
- How an invalid write can corrupt nearby memory
- Why undefined behavior does not necessarily cause a crash
- How to observe memory corruption using GDB
- How stack layout affects what an out-of-bounds write can corrupt

This challenge does not attempt to overwrite a return address or execute arbitrary code.

The focus is on understanding the memory corruption itself.

---

## Prerequisites

This challenge assumes that the following tools are installed:

- GCC
- GDB
- Make
- Linux or WSL
- VS Code with the WSL environment used by the project

If the required tools are not installed on Ubuntu/WSL:

```bash
sudo apt update
sudo apt install build-essential gdb
Challenge Source

The complete source code for this challenge is:

#include <stdio.h>

int main(void)
{
    int buffer[4];
    int secret = 0x12345678;

    printf("buffer: %p\n", (void *)buffer);
    printf("secret: 0x%x\n", secret);

    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;
    buffer[3] = 4;

    buffer[4] = 0x41414141;

    printf("buffer[0]: %d\n", buffer[0]);
    printf("buffer[1]: %d\n", buffer[1]);
    printf("buffer[2]: %d\n", buffer[2]);
    printf("buffer[3]: %d\n", buffer[3]);

    return 0;
}

The intentionally vulnerable line is:

buffer[4] = 0x41414141;

The array was declared as:

int buffer[4];

This creates four valid array elements:

buffer[0]
buffer[1]
buffer[2]
buffer[3]

There is no valid buffer[4].

1. Understanding the Array

The declaration:

int buffer[4];

creates an array containing four int objects.

On the system being used for this lab, an int is four bytes.

Therefore:

4 elements × 4 bytes = 16 bytes

Conceptually, the array looks like:

buffer
  |
  v
+-------------------+
| buffer[0]         | 4 bytes
+-------------------+
| buffer[1]         | 4 bytes
+-------------------+
| buffer[2]         | 4 bytes
+-------------------+
| buffer[3]         | 4 bytes
+-------------------+
        16 bytes

The valid indexes are:

0
1
2
3

Index 4 is outside the array.

2. Why buffer[4] Is Vulnerable

The following statement is invalid:

buffer[4] = 0x41414141;

The program is attempting to access an element that does not exist.

This is an out-of-bounds write.

In C, ordinary arrays do not automatically perform runtime bounds checking.

The compiler knows that buffer contains four elements, but the generated machine code does not generally stop execution simply because an index is outside the array.

Instead, the index is used to calculate an address.

Conceptually:

buffer[0] → base + 0 bytes
buffer[1] → base + 4 bytes
buffer[2] → base + 8 bytes
buffer[3] → base + 12 bytes
buffer[4] → base + 16 bytes

buffer[4] therefore refers to the memory immediately after the array.

That memory is not part of the array.

3. Build the Challenge

All commands in this section should be executed from the root of the VulnerabilitiesLab project.

Build the project:

make

The executable should be created at:

bin/challenge02

Verify that it exists:

ls -l bin/challenge02
4. Run the Program Normally

Run:

./bin/challenge02

You should see output similar to:

buffer: 0x7fff003f6ee0
secret: 0x12345678
buffer[0]: 1
buffer[1]: 2
buffer[2]: 3
buffer[3]: 4

The exact address will vary.

Notice that the program may run without crashing.

This is an important part of the challenge.

The statement:

buffer[4] = 0x41414141;

is invalid, but invalid C behavior does not necessarily cause an immediate segmentation fault.

5. Start GDB

Start GDB using the Makefile:

make debug02

Alternatively:

gdb ./bin/challenge02

Inside GDB, set a breakpoint at main:

break main

Start the program:

run

GDB should stop at the beginning of main.

6. Examine the Source

Use:

list

You should see the source code around main.

The important section is:

int buffer[4];
int secret = 0x12345678;

...

buffer[0] = 1;
buffer[1] = 2;
buffer[2] = 3;
buffer[3] = 4;

buffer[4] = 0x41414141;

The vulnerability occurs at:

buffer[4] = 0x41414141;
7. Examine the Array

Use:

print buffer

Before the initialization statements have executed, the array has not yet been initialized by the C program.

After stepping through the initialization statements, use:

print buffer

You should see:

$1 = {1, 2, 3, 4}

The four values correspond to:

buffer[0] = 1
buffer[1] = 2
buffer[2] = 3
buffer[3] = 4
8. Determine the Array Size

Use:

print sizeof(buffer)

Expected:

$2 = 16

Now check the size of one element:

print sizeof(buffer[0])

Expected:

$3 = 4

Therefore:

4 elements × 4 bytes = 16 bytes

You can also determine the number of elements using:

print sizeof(buffer) / sizeof(buffer[0])

Expected:

$4 = 4

This confirms that the array contains four elements.

9. Examine the Address of Each Element

Use:

print &buffer[0]
print &buffer[1]
print &buffer[2]
print &buffer[3]

An example execution may show:

&buffer[0] = 0x7fffffffd930
&buffer[1] = 0x7fffffffd934
&buffer[2] = 0x7fffffffd938
&buffer[3] = 0x7fffffffd93c

Your addresses will probably be different.

The important observation is the spacing:

buffer[0] → +0 bytes
buffer[1] → +4 bytes
buffer[2] → +8 bytes
buffer[3] → +12 bytes

Each int occupies four bytes.

10. Examine the Address of buffer[4]

Now ask GDB for the address of the invalid element:

print &buffer[4]

You may see:

$5 = (int *) 0x7fffffffd940

For example:

buffer[3] → 0x7fffffffd93c
buffer[4] → 0x7fffffffd940

The difference is four bytes.

This demonstrates that the compiler can calculate an address for buffer[4].

However, calculating the address does not make the access valid according to the C language.

11. Examine the Memory at buffer[4]

Use:

x/wx &buffer[4]

You may see:

0x7fffffffd940: 0x00000000

The exact value depends on the current execution.

The important observation is that the address exists as an address that the debugger and processor can access.

C's array boundary is a language-level rule.

It is not a physical barrier in memory.

12. Demonstrate an Out-of-Bounds Read

Use:

print buffer[4]

You may see:

$6 = 0

The value is not important.

What matters is that the program is attempting to interpret memory beyond the array as another int.

This is an out-of-bounds read.

The operation is undefined behavior.

13. Examine the Array and the Memory Immediately After It

Use:

x/6wx buffer

Before the vulnerable write executes, you may see something similar to:

0x7fffffffd930: 0x00000001  0x00000002  0x00000003  0x00000004
0x7fffffffd940: 0x00000000  0x00000000

The first four words are:

buffer[0] = 1
buffer[1] = 2
buffer[2] = 3
buffer[3] = 4

The next word is memory immediately after the array.

It is not a fifth valid array element.

14. Set a Breakpoint at the Vulnerable Write

Find the source line containing:

buffer[4] = 0x41414141;

Set a breakpoint on that line.

For example, if the source line is 16:

break challenge02/challenge02.c:16

If your line number differs, use the line number shown by GDB for the buffer[4] statement.

Restart the program:

run

If GDB asks whether you want to restart the program, answer:

y

GDB should stop before executing the vulnerable assignment.

You should see something similar to:

Breakpoint 2, main () at challenge02/challenge02.c:16
16     buffer[4] = 0x41414141;

The important point is that the assignment has not executed yet.

15. Examine the Array Before the Vulnerable Write

Use:

print buffer

Expected:

$1 = {1, 2, 3, 4}

Now inspect six words:

x/6wx buffer

Example:

0x7fffffffd930: 0x00000001  0x00000002  0x00000003  0x00000004
0x7fffffffd940: 0x00000000  0x00000000

Record the address and value immediately after the array.

For this example:

buffer[4] address = 0x7fffffffd940
value             = 0x00000000
16. Execute the Vulnerable Write

The current source line is:

buffer[4] = 0x41414141;

Execute the current source-level statement:

next

GDB should move to the next source line.

The vulnerable write has now executed.

17. Examine the Memory After the Write

Run:

x/6wx buffer

You should now see something similar to:

0x7fffffffd930: 0x00000001  0x00000002  0x00000003  0x00000004
0x7fffffffd940: 0x41414141  0x00000000

Compare this with the previous output.

Before:

0x7fffffffd940: 0x00000000

After:

0x7fffffffd940: 0x41414141

The program has written data outside the bounds of the array.

This is the memory corruption demonstrated by Challenge 02.

18. Verify the Corrupted Memory Through GDB

Use:

print buffer[3]

Expected:

$2 = 4

Now:

print buffer[4]

You may see:

$3 = 1094795585

That is the decimal representation of:

0x41414141

Display it directly as hexadecimal:

print/x buffer[4]

Expected:

$4 = 0x41414141

This confirms that the memory at the out-of-bounds location contains the value written by the vulnerable statement.

19. Understand 0x41414141

The value:

0x41414141

is commonly used as a recognizable memory-corruption test value.

Hexadecimal 0x41 corresponds to the ASCII character:

A

Therefore:

0x41 0x41 0x41 0x41

corresponds to:

AAAA

This makes the value easy to recognize when looking through raw memory.

It is not a special CPU value.

It is simply a recognizable test pattern.

20. Check secret

The program contains:

int secret = 0x12345678;

Check its value:

print secret

Expected:

$5 = 305419896

The decimal value:

305419896

is:

0x12345678

Check its address:

print &secret

For example:

$6 = (int *) 0x7fffffffd92c

Then inspect its memory:

x/wx &secret

Expected:

0x7fffffffd92c: 0x12345678

In the observed execution, secret remains unchanged.

21. Why Was secret Not Overwritten?

It is important not to assume that an out-of-bounds write automatically overwrites the next C variable.

The compiler determines the actual layout of local variables in the stack frame.

In one observed execution, the addresses were:

secret      = 0x7fffffffd92c

buffer[0]   = 0x7fffffffd930
buffer[1]   = 0x7fffffffd934
buffer[2]   = 0x7fffffffd938
buffer[3]   = 0x7fffffffd93c
buffer[4]   = 0x7fffffffd940

The memory relationship was:

secret
  |
  v
0x...d92c

buffer[0]
  |
  v
0x...d930

buffer[1]
  |
  v
0x...d934

buffer[2]
  |
  v
0x...d938

buffer[3]
  |
  v
0x...d93c

buffer[4] destination
  |
  v
0x...d940

Therefore, in this execution, writing to buffer[4] did not overwrite secret.

This demonstrates why memory-corruption analysis must be based on the actual memory layout.

22. Inspect the Stack Frame

Use:

info frame

An observed execution produced information similar to:

Stack level 0, frame at 0x7fffffffd960:
 rip = 0x5555555551fb in main (challenge02/challenge02.c:19);
 saved rip = 0x7ffff7c2a601
 Arglist at 0x7fffffffd950
 Locals at 0x7fffffffd950
 Saved registers:
  rbp at 0x7fffffffd950, rip at 0x7fffffffd958

Your addresses will likely be different.

The important information is the relationship between:

buffer[4]
saved RBP
saved RIP

For example:

buffer[4] → 0x7fffffffd940
saved RBP → 0x7fffffffd950
saved RIP → 0x7fffffffd958

In this execution, the vulnerable write is below the saved frame information.

It does not overwrite the saved return address.

23. Examine $rbp

Use:

print/x $rbp

You may see:

$7 = 0x7fffffffd950

The value will vary between executions.

RBP is commonly used as a frame pointer in debug builds.

It can help us understand the layout of the current stack frame.

24. Examine $rsp

Use:

print/x $rsp

You may see:

$8 = 0x7fffffffd920

RSP is the current stack pointer.

The stack grows toward lower addresses on the x86-64 Linux system used for this lab.

Do not rely on one particular absolute address.

Focus on the relative positions of objects.

25. Examine Raw Stack Memory

Use:

x/20wx $rsp

An observed execution produced:

0x7fffffffd920: 0x00000000  0x00000000  0x00000000  0x12345678
0x7fffffffd930: 0x00000001  0x00000002  0x00000003  0x00000004
0x7fffffffd940: 0x41414141  0x00000000  0x4d921700  0xb1d438c2
0x7fffffffd950: 0xffffda00  0x00007fff  0xf7c2a601  0x00007fff
0x7fffffffd960: 0xffffda40  0x00007fff  0xffffda88  0x00007fff

The exact values will vary.

The important observation is:

0x7fffffffd930:
    1
    2
    3
    4

0x7fffffffd940:
    0x41414141

The recognizable value appears immediately after the four legitimate array elements.

26. Examine the Stack as 8-Byte Values

GDB can display the same memory using eight-byte units.

Use:

x/12gx $rsp

An observed execution produced:

0x7fffffffd920: 0x0000000000000000      0x1234567800000000
0x7fffffffd930: 0x0000000200000001      0x0000000400000003
0x7fffffffd940: 0x0000000041414141      0x8d7e634ce88b7400
0x7fffffffd950: 0x00007fffffffda00      0x00007ffff7c2a601
0x7fffffffd960: 0x00007fffffffda40      0x00007fffffffda88
0x7fffffffd970: 0x00000001f7fc0000      0x0000555555555169

This is the same underlying memory displayed using a different unit size.

27. Understanding the GDB x Command

The general form of the GDB examine command is:

x/[count][format][unit] address

For example:

x/6wx buffer

means:

6 → display six values
w → display words (4 bytes)
x → display in hexadecimal

Another example:

x/12gx $rsp

means:

12 → display twelve values
g → display giant words (8 bytes)
x → display in hexadecimal

Useful commands for this challenge include:

x/wx address
x/wd address
x/gx address
x/6wx address
x/12gx address
28. Inspect the Saved Return Address

info frame identifies the location of the saved return address.

For example:

saved rip at 0x7fffffffd958

The exact address will vary.

You can inspect it with:

x/gx 0x7fffffffd958

Use the actual saved-RIP address reported by your info frame output.

In the observed execution:

buffer[4] destination → 0x7fffffffd940
saved RIP             → 0x7fffffffd958

These are different locations.

Therefore, the buffer[4] write did not overwrite the saved return address.

This challenge intentionally stops before demonstrating control-flow corruption.

29. Important Rule When Reading Raw Stack Memory

Do not automatically assign a meaning to every value displayed by:

x/20wx $rsp

Raw memory does not contain labels such as:

secret
buffer
saved RBP
saved RIP

Those labels come from the debugger's understanding of the program and its stack frame.

Memory immediately outside a C object could contain:

padding
alignment space
compiler-generated temporary storage
other local data
saved registers
stack-frame information
unrelated values

Only identify a memory location as a specific object when there is evidence supporting that interpretation.

30. Repeat the Before-and-After Experiment

Restart the program:

run

If GDB asks whether to restart, enter:

y

Stop at:

buffer[4] = 0x41414141;

Before executing the statement:

x/6wx buffer

You should see something similar to:

0x7fffffffd930: 0x00000001  0x00000002  0x00000003  0x00000004
0x7fffffffd940: 0x00000000  0x00000000

Now execute:

next

Then inspect the memory again:

x/6wx buffer

You should now see:

0x7fffffffd930: 0x00000001  0x00000002  0x00000003  0x00000004
0x7fffffffd940: 0x41414141  0x00000000

The exact addresses may change.

The important change is:

Before:

0x00000000

After:

0x41414141
31. Verify the Array Boundary

Run:

print sizeof(buffer) / sizeof(buffer[0])

Expected:

$9 = 4

Therefore:

Valid indexes:
0
1
2
3

And:

Invalid index:
4

This is the central vulnerability of Challenge 02.

32. Why the Vulnerability Exists

The programmer wrote:

int buffer[4];

but later accessed:

buffer[4]

The program assumes that the requested index is valid.

C does not automatically stop the program when it is not.

Conceptually, the compiler calculates something equivalent to:

address = base_address + index × sizeof(int)

For:

index = 4
sizeof(int) = 4

the offset is:

4 × 4 = 16 bytes

So:

buffer[4]

targets:

base address + 16 bytes

That address is outside the 16-byte array.

33. Undefined Behavior

The C language defines accessing an array outside its valid bounds as undefined behavior.

Undefined behavior does not mean that one particular outcome is guaranteed.

The program could:

appear to work
modify another object
corrupt stack data
produce unexpected results
crash immediately
crash later
behave differently after recompilation

Therefore:

No crash

does not mean:

No vulnerability

The invalid write itself is the vulnerability.

34. Why an Out-of-Bounds Write Is Important

An out-of-bounds write can modify memory belonging to something other than the intended array.

Depending on the program's layout, this could potentially affect:

another local variable
control data
pointers
function state
other stack data
heap metadata
other application data

This challenge deliberately uses a simple write so that the memory corruption can be observed without immediately introducing control-flow manipulation.

35. Why buffer[4] Does Not Automatically Mean secret

It is important to avoid thinking about C variables as though they are guaranteed to be arranged in source-code order.

The source contains:

int buffer[4];
int secret = 0x12345678;

but the compiler is responsible for producing the actual machine-level layout.

The resulting memory arrangement can depend on:

compiler
compiler version
architecture
optimization level
debug settings
ABI
alignment
other generated code

Therefore, always inspect the actual layout with GDB.

36. Why Addresses Change Between Runs

Modern Linux systems use address-space randomization.

As a result, stack addresses may change when the program is restarted.

For example:

Run 1:
buffer = 0x7fffffffd930

and:

Run 2:
buffer = 0x7fffffffd8a0

The exact address is not the important part.

Instead, focus on relationships such as:

buffer[0] → base + 0
buffer[1] → base + 4
buffer[2] → base + 8
buffer[3] → base + 12
buffer[4] → base + 16
37. Memory Layout Observed in This Challenge

One observed execution produced approximately:

secret      = 0x7fffffffd92c

buffer[0]   = 0x7fffffffd930
buffer[1]   = 0x7fffffffd934
buffer[2]   = 0x7fffffffd938
buffer[3]   = 0x7fffffffd93c
buffer[4]   = 0x7fffffffd940

saved RBP   = 0x7fffffffd950
saved RIP   = 0x7fffffffd958

This can be visualized as:

Lower addresses
       |
       v

0x...d92c    secret
             |
0x...d930    buffer[0]
             |
0x...d934    buffer[1]
             |
0x...d938    buffer[2]
             |
0x...d93c    buffer[3]
             |
0x...d940    buffer[4] destination
             |
             | other stack memory
             |
0x...d950    saved RBP
             |
0x...d958    saved RIP

       ^
       |
Higher addresses

These exact addresses are only an example from one execution.

Do not hard-code them into future analysis.

38. What We Have Proven

The GDB investigation has demonstrated the following:

The array contains four elements:

buffer[0]
buffer[1]
buffer[2]
buffer[3]

The next calculated address is:

buffer[4]

which is outside the array.

Before the vulnerable statement executes:

buffer[4] location → 0x00000000

After:

buffer[4] = 0x41414141;

the memory contains:

buffer[4] location → 0x41414141

Therefore, we have directly observed an out-of-bounds memory write.

Key Concepts
Array Bounds

For:

int buffer[4];

the valid indexes are:

0 through 3

Index 4 is invalid.

Array Indexing

Array indexing is based on the address of the first element and the size of each element.

For four-byte integers:

buffer[0] → base + 0
buffer[1] → base + 4
buffer[2] → base + 8
buffer[3] → base + 12
buffer[4] → base + 16
Out-of-Bounds Read

An expression such as:

buffer[4]

attempts to read memory outside the array.

That is an out-of-bounds read.

Out-of-Bounds Write

An expression such as:

buffer[4] = 0x41414141;

attempts to write memory outside the array.

That is an out-of-bounds write.

Memory Corruption

When an out-of-bounds write changes memory outside the intended object, it can result in memory corruption.

In this challenge:

buffer[4]
    |
    v
0x41414141

is the direct example of memory corruption.

Undefined Behavior

An out-of-bounds array access is undefined behavior.

The C language does not define what the program must do after the invalid operation.

Therefore, the result cannot be predicted reliably from the C source alone.

Stack Layout

The compiler determines how local variables and other data are arranged in the stack frame.

The source-code order of variables does not guarantee their exact memory layout.

GDB Memory Examination

Useful GDB commands from this challenge include:

print buffer
print &buffer[0]
print &buffer[1]
print &buffer[2]
print &buffer[3]
print &buffer[4]

print sizeof(buffer)
print sizeof(buffer[0])
print sizeof(buffer) / sizeof(buffer[0])

x/wx &buffer[4]
x/6wx buffer
x/20wx $rsp
x/12gx $rsp

info frame

print/x $rbp
print/x $rsp
Completion Checklist

Challenge 02 is complete when you can explain all of the following:

 buffer contains four elements
 The valid indexes are 0 through 3
 buffer[4] is outside the array
 An int occupies four bytes on this system
 The array occupies 16 bytes
 Consecutive array elements are four bytes apart
 buffer[4] is located 16 bytes from the start of the array
 C does not automatically perform bounds checking on this array
 buffer[4] can produce an out-of-bounds read
 buffer[4] = 0x41414141 performs an out-of-bounds write
 0x41414141 is an easily recognizable test value
 The value 0x41414141 can be observed in memory with GDB
 An out-of-bounds write does not necessarily crash the program
 Undefined behavior does not have a guaranteed result
 secret is not necessarily overwritten
 The exact stack layout depends on the generated program
 Stack addresses can change between executions
 info frame can be used to inspect the current stack frame
 The saved return address can be located using GDB
 The vulnerable write in this challenge does not overwrite the saved return address
 Raw stack memory should not be assigned meanings without evidence
Challenge 02 Takeaway

The vulnerability is:

int buffer[4];

...

buffer[4] = 0x41414141;

The array contains four valid elements, but the program writes to a fifth element that does not exist.

The processor can still calculate and access the corresponding memory address.

The C language, however, considers that access invalid.

GDB allows us to observe the result directly.

Before the write:

buffer[0]   0x00000001
buffer[1]   0x00000002
buffer[2]   0x00000003
buffer[3]   0x00000004
buffer[4]   0x00000000

After the write:

buffer[0]   0x00000001
buffer[1]   0x00000002
buffer[2]   0x00000003
buffer[3]   0x00000004
buffer[4]   0x41414141

The critical lesson is:

The absence of a crash does not mean the absence of a vulnerability.

The vulnerability is the fact that the program allowed a write to occur outside the bounds of the intended array.

This is the foundation for more serious memory-corruption vulnerabilities.

Challenge 02 Summary

The progression of this challenge is:

C array
   |
   v
int buffer[4]
   |
   v
Four valid elements
   |
   +--> buffer[0]
   +--> buffer[1]
   +--> buffer[2]
   +--> buffer[3]
   |
   v
Array boundary
   |
   v
buffer[4]
   |
   v
Out-of-bounds access
   |
   v
0x41414141 written
   |
   v
Memory corruption