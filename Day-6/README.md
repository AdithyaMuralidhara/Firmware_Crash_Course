# MISRA C

## Critical MISRA Rules to Memorize

`1. No Dynamic Memory Allocation (Rule 18.7)`

The Rule: Functions like malloc(), calloc(), and free() are strictly forbidden.
The Why: Dynamic memory is non-deterministic. We cannot guarantee a "heap" will have space during a high-speed driving maneuver. All memory must be allocated statically at compile time.

`2. Use Fixed-Width Integers (Rule 4.6)`

The Rule: Do not use basic types like int, long, or short. Use <stdint.h> types.
The Why: On an 8-bit AVR, an int might be 16 bits; on your STM32, it’s 32 bits. Using uint32_t or int16_t ensures it is identical across all hardware.

`3. No "Magic Numbers" (Rule 2.1)`

The Rule: All literal values (except 0 and 1) must be defined as constants or macros.
The Why: Writing USART1->BRR = 0x1D4C; is a nightmare to debug. Writing USART1->BRR = BAUD_9600_VAL; tells exactly what is happening.

`4. Braces for Every Control Statement (Rule 15.6)`

The Rule: if, else, while, and for statements must always use curly braces { }, even for a single line.
The Why: It prevents the "Apple SSL Bug" (goto fail), where a stray line of code looks like it's inside an if block but actually executes every time.

`5. No Assignments in Loop/If Expressions (Rule 13.4)`
The Rule: You cannot do if (a = b).
The Why: It is far too easy to type = (assignment) when we meant == (comparison). MISRA forces us to separate the logic.

`6.All "if-else if" chains must end with an "else" (Rule 15.7)`
The Rule: Every if ... else if sequence must have a final else statement, even if it’s just a comment like /* Else not reached */.

`7. No Function Pointers (Rule 11.1 / 11.6)`
The Rule: Avoid using pointers to functions unless necessary.
The Why: If a function pointer gets corrupted, the CPU could jump to a random memory address and start executing garbage code.
