# Day-7 Summary of Phase 1 (Foundations)

| Topic | Key Concepts Implemented |
| :--- | :--- |
| **Advanced C for Embedded** | Bitwise logic (`OR`, `&`, `^`, `~`), `volatile` vs `const`, Circular Buffers (FIFO). |
| **Bare-Metal GPIO & RCC** | Memory Map navigation, RCC (Reset and Clock Control), GPIO Output/Input modes without HAL. |
| **Bare-Metal USART** | Baud rate calculation, Status Register polling, TX/RX data registers. |
| **MISRA C Guidelines** | Automotive safety standards, fixed-width integers. |
| **Interrupts (NVIC)** | Vector Table understanding, ISR implementation, Priority grouping. |

---

## Key Learnings

### 1. Register-Level Peripheral Control
Instead of `HAL_UART_Transmit`, memory address manipulation was done using STM32 Datasheet and Reference Manual.

**Flowchart:**
1.  **Clock Enable:** Enable peripheral clock via `RCC->AHB1ENR` or `RCC->APB1ENR`.
2.  **Pin Muxing:** Configure GPIO Alternate Functions (AF) in `GPIOx->AFR`.
3.  **Configuration:** Set Control Registers (CR) and Baud Rate Registers (BRR).
4.  **Enable:** Set the specific Enable bit (UE) in the Control Register.

### 2. The NVIC & Interrupt
Understanding how the Cortex-M4 handles asynchronous events. 
* **Vector Table:** Analyzed the startup file to understand how the CPU jumps to ISRs.
* **Priorities:** Configured Preemption and Sub-priority fields in the NVIC.
* **ISR Rules:** Implemented lean ISRs that only clear flags and buffer data, deferring heavy processing to the main loop.

### 3. MISRA C:2012 Compliance
Studied the main rules of MISRA C

* **Rule 18.7:** Zero use of dynamic memory (`malloc`/`free`) to prevent heap fragmentation.
* **Rule 4.6:** Exclusive use of fixed-width integers (`uint32_t`, `int16_t`) instead of generic `int` or `long`.
* **Rule 15.6:** Mandatory use of braces `{ }` for all control structures, even single-line statements.
* **Rule 11.4:** Safe conversion between pointers and integer types.
