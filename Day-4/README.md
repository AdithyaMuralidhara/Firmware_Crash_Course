# Day-7 UART Driver

The goal of Day 7 was to establish communication between the STM32 microcontroller and a PC without using any HAL (Hardware Abstraction Layer) libraries or `printf`.

The code sets up the system clock to 72MHz, configures GPIO pins for Alternate Functions, and implements an "Echo" feature where the microcontroller receives a character and immediately transmits it back.

## Hardware Configuration
* **Microcontroller:** STM32F103 (Blue Pill)
* **Peripheral:** USART1
* **Pins:**
    * **PA9 (TX):** Alternate Function Output Push-Pull
    * **PA10 (RX):** Input Floating
* **Clock Speed:** 72 MHz (HSE 8MHz -> PLL x9)
* **Baud Rate:** 115200

## Core Concepts Learned

### 1. Baud Rate Calculation
UART is asynchronous, meaning there is no shared clock line. Both devices must agree on the speed (Baud Rate) to sample bits correctly.
* **Formula:** `USARTDIV = f_CK / (16 * BaudRate)`
* **Calculation for 115200 at 72MHz:**
    * Result: 39.0625
    * Mantissa (Integer): 39 (`0x27`)
    * Fraction (Decimal): 0.0625 * 16 = 1 (`0x1`)
    * **BRR Register Value:** `0x271`

### 2. GPIO Alternate Functions
To use a pin for UART, it must be disconnected from the standard GPIO controller and connected to the internal UART hardware.
* **Register:** `GPIOA_CRH` (Control Register High) is used because PA9 and PA10 are in the upper half of the port.
* **Logic:** Each pin occupies **4 bits** in the register.
    * PA9 (TX) needs `0xB` (Output 50MHz, Alt Func Push-Pull).
    * PA10 (RX) needs `0x4` (Input Floating).
* **Safety:** Always clear the 4 bits (`&= ~`) before setting new ones to avoid overlapping settings.

### 3. The PLL "Plus Two" Rule
When setting the PLL Multiplier in `RCC_CFGR`, the binary value does not equal the multiplier directly.
* **Rule:** `Multiplier = RegisterValue + 2`
* **Target:** x9 Multiplier.
* **Math:** `9 = RegisterValue + 2` -> `RegisterValue = 7`.
* **Code:** Write `0x7` (binary `0111`) to the PLLMUL bits, not `0x9`.

## Implementation Details

### Register Map
* **USART_SR (Status Register):** Used to check if data is ready to be read (`RXNE`) or if the hardware is ready to send (`TXE`).
* **USART_DR (Data Register):** The "mailbox" for sending and receiving bytes.
* **USART_BRR (Baud Rate Register):** Holds the clock divisor value.
* **USART_CR1 (Control Register):** Enables the UART, Transmitter, and Receiver.

### Transmission Logic
1.  Wait until the **TXE** (Transmit Empty) flag is set.
2.  Write data to **DR**.

### Reception Logic
1.  Wait until the **RXNE** (Read Not Empty) flag is set.
2.  Read data from **DR**.
3.  *Note:* Reading the DR automatically clears the RXNE flag.

## Frequently Asked Questions (My Learning Notes)

**Q: Why do we need `volatile` for pointers?**
A: `volatile` tells the compiler that the value at this address can change at any time (by hardware), so it should not optimize the code by caching the value. Without this, the compiler might assume the status flags never change and freeze the program.

**Q: Why use `0x7` for the PLL Multiplier instead of `9`?**
A: The STM32 hardware maps the binary value `0000` to "x2". Therefore, to get "x9", we must write `7` (`0111`). Writing `9` (`1001`) would result in "x11", which creates an 88MHz clock and overclocks the system.

**Q: Why was my GPIO bit-shifting logic overlapping?**
A: The `CRH` register allocates 4 bits per pin.
* Pin 9 starts at bit 4.
* Pin 10 starts at bit 8.
* Shifting `(1<<10)` affects Pin 10, but shifting `(1<<6)` affects Pin 9. It is cleaner to define the full 4-bit nibble and shift it to the correct start position (e.g., `0xB << 4`).

**Q: How do we fix the "Blocking" logic?**
A: Instead of using `while` loops that trap the CPU until an event happens, professional firmware checks flags inside a main loop. However, for this bare-metal driver, we used a simple polling loop:
`while (!(StatusRegister & Flag));`
This waits specifically while the flag is **not** set.
