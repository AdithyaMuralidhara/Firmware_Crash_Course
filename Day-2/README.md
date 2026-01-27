
# Day 2: Bare-Metal STM32 & Register Access

## Goal

To control the STM32F103C8T6 hardware directly by accessing registers instead of using the HAL library.

## 1. Memory Mapping

To access a specific address in memory, we use "`*(volatile uint32_t *)0x400021000`". By casting a hex address to a `volatile uint32_t *`, we can talk to the hardware directly.


## 2. The Clock Tree (RCC)

Peripherals are inactive by default to save power. To use them, we must:

1. **Enable the Clock:** Flip the bit in the `RCC` register.
2. **Wait for Stability:** Use a `while` loop to check if the clock/PLL is "Ready."

## 3. Register Configuration

To blink an LED on **PC13** at **72MHz**:

* **RCC_CR & RCC_CFGR:** Enabled the HSE (External Crystal) and set the PLL multiplier to 9x (9x8MHz = 72MHz).
* **FLASH_ACR:** Set 2 wait states because the Flash memory is slower than the 72MHz CPU.
* **GPIOC_CRH:** Configured Pin 13 as a "General Purpose Output".
* **GPIOC_ODR:** Used this register to toggle the bit (0 or 1) that controls the physical pin voltage.

## Memory Addresses Used (STM32F103)

| Register | Address | Purpose |
| --- | --- | --- |
| **RCC_CR** | `0x40021000` | Clock Control (HSE/PLL Enable) |
| **RCC_CFGR** | `0x40021004` | Clock Configuration (Multiplier/Source) |
| **FLASH_ACR** | `0x40022000` | Flash Latency (Wait States) |
| **GPIOC_CRH** | `0x40011004` | Pin Mode Configuration (Pin 13) |
| **GPIOC_ODR** | `0x4001100C` | Output Data (LED On/Off) |


<img width="679" height="864" alt="Screenshot 2026-01-27 123044" src="https://github.com/user-attachments/assets/53179352-da65-4664-b629-4dbc79437626" />


<img width="860" height="607" alt="image" src="https://github.com/user-attachments/assets/ce9dcbb9-9078-4396-93c8-47fa6a7d1aa0" />


## Code Logic Breakdown

1. **Power Up:** Enable HSE and wait for `HSERDY`.
2. **Speed Up:** Configure PLL for 72MHz and wait for `PLLRDY`.
3. **Setup Pin:** Enable GPIOC clock and set Pin 13 to Output mode.
4. **Loop:** Toggle the 13th bit of the `ODR` and use a `volatile` for-loop for delay.

## Things to Remember

* **Base + Offset:** Always calculate the register address by adding the offset from the datasheet to the Peripheral Base address.
* **Reset Values:** Before setting bits in a register (like `CRH`), it is safer to clear them first to avoid unexpected behavior.
* **Bus Architecture:** GPIOs on the F103 live on the **APB2** bus.
