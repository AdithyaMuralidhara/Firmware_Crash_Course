# Day 3: External Interrupts (EXTI) and NVIC


Instead of polling(Checking Continously) a button state in a `while(1)` loop, the CPU sleeps until a physical button press triggers a hardware interrupt.

**Goal:** Toggle an LED (PC13) when a button (PA0) is pressed using the EXTI (External Interrupt) peripheral and NVIC (Nested Vectored Interrupt Controller).


### 1. Polling vs. Interrupts

* **Polling:** The CPU constantly checks a register flag (`if(IDR & 1)`). This wastes CPU cycles and reacts slowly.
* **Interrupts:** The CPU does other work (or sleeps). When the trigger occurs, It jumps to ISR.

### 2. Signal 

The signal travels through three distinct blocks:

1. **GPIO (Port A):** The physical pin detects the voltage change.
2. **AFIO (Alternate Function I/O):** Acts as a MUX. It routes "Port A Pin 0" to "EXTI Line 0". As all ports (A,B,C) has Port 0 but EXTI has only 1 line for Port 0, AFIO routes it.
3. **EXTI (External Interrupt Controller):** Detects the edge (Rising/Falling) and sets a "Pending" flag.
4. **NVIC (Nested Vectored Interrupt Controller):** The CPU's interrupt manager. If EXTI signals it, NVIC forces the CPU to jump to the Vector Table address.

## Register Map & Configuration

### Clock Configuration (`RCC_APB2ENR`)

We must enable clocks for three peripherals:

* **IOPCEN (Bit 4):** Port C (for LED).
* **IOPAEN (Bit 2):** Port A (for Button).
* **AFIOEN (Bit 0):** Alternate Function IO. **Crucial:** Without this, the EXTI mapping logic won't work.

### GPIO Configuration (`GPIOA_CRL`)

PA0 must be configured as **Input Floating** (Mode `00`, CNF `01` -> Hex `0x4`). Mode 00 -> Input and CNF 01 -> Floating IP, Mostly for Buttons 

* **Important:** Always **CLEAR** the register bits (`&= ~0xF`) before setting them. OR-ing (`|=`) on a non-zero register can result in accidental Output modes.

### AFIO Mapping (`AFIO_EXTICR1`)

Maps the physical port to the EXTI line.

* By default, EXTI0 is mapped to Port A. However, it is best practice to explicitly clear/set the bits in `AFIO_EXTICR1`.

### EXTI Setup

* **`EXTI_IMR` (Interrupt Mask Register):** Set Bit 0 to `1` to unmask (enable) the interrupt.
* **`EXTI_FTSR` (Falling Trigger Selection):** Set Bit 0 to `1` to trigger on a Falling Edge (High -> Low).

### NVIC Setup (`NVIC_ISER0`)

* **Address:** `0xE000E100`
* **Logic:** Enables the interrupt at the core level.
* **STM32F1 Position:** EXTI0 is at position **6** in the vector table. Write `1 << 6`.

## The Interrupt Service Routine (ISR)

The function `EXTI0_IRQHandler` is special. The linker looks for this specific name in the startup file.

### Critical Implementation Details

1. **Check the Flag:** Verify that EXTI Line 0 actually triggered the interrupt.
2. **Clear the Flag:** We **must** clear the Pending Bit (`EXTI_PR`) inside the ISR.
* **Mechanism:** Write `1` to the bit to clear it.
* **Failure:** If we write `0` (or forget to clear it), the NVIC sees the interrupt is still pending and will infinitely loop.



## Common Pitfalls & Lessons Learned

### 1. Assuming register is 0x00

When configuring GPIO modes, never assume the register is `0x00` by default.

* **Bad:** `reg |= 0x4;` (If reg was `0x1`, result is `0x5` -> Output mode).
* **Good:** `reg &= ~(0xF); reg |= 0x4;` (Safely clears old settings first).

### 2. The `volatile` Keyword

Any variable shared between the main loop and an ISR must be declared `volatile`. This prevents the compiler from optimizing out reads to that variable, ensuring the main loop always sees the latest value updated by the interrupt.

### 3. NVIC vs. Peripheral Enable

Enabling the EXTI (Peripheral) is not enough. We must also enable the specific IRQ channel in the NVIC (Core). Both switches must be "ON" for the interrupt to fire.
