# Day 5: Direct Memory Access (DMA)


Direct Memory Access (DMA) is a hardware feature that allows data to be moved from one place to another (e.g., from a peripheral to RAM) without using the CPU.

* **Without DMA:** The CPU must manually read every byte from the UART register and write it to a buffer. This consumes processing cycles and prevents the CPU from doing other tasks.
* **With DMA:** The DMA controller handles the transfer in the background. The CPU is only notified via an interrupt when the entire transaction (e.g., a full 16-byte message) is complete.


## 2. Hardware Architecture

The STM32F103 uses a **Bus Matrix**. Both the CPU and the DMA controller are "Masters" on this bus.

* They share access to memory (SRAM) and peripherals.
* If both try to access the same resource simultaneously, the Bus Matrix acts as a traffic police, usually stealing a few cycles from the CPU to let the DMA finish.
* **DMA1 Channel 5** can be connected to **USART1_RX** requests and **DMA1 Channel 4** can be connected to **USART1_TX**

## 3. Key Registers

To configure DMA without a library, we must manipulate these specific registers.

### A. RCC_AHBENR (DMA Clock)

* DMA is located on the **AHB** (Advanced High-performance Bus).
* We must enable the clock for DMA1 (Bit 0) before configuring any other DMA registers.

### B. DMA_CPARx (Peripheral Address)

* **CPAR = Peripheral Address Register.**
* This is the "Source" address. It tells the DMA where to fetch data from.
* For UART, this is the fixed physical address of the `USART_DR` (Data Register).

### C. DMA_CMARx (Memory Address)

* **CMAR = Memory Address Register.**
* This is the "Destination" address. It tells the DMA where to store the data.
* This is usually the address of an array (buffer) in RAM.

### D. DMA_CNDTRx (Number of Data)

* **CNDTR = Counter Register.**
* We set the number of bytes to transfer (e.g., 16).
* This value **decrements** after every byte transferred.
* When it reaches 0, the transfer stops (or reloads if in Circular Mode).

### E. DMA_CCRx (Configuration Control)

This is the "brain" of the DMA channel. Key bits include:

* **EN (Enable):** Turns the channel on/off.
* **MINC (Memory Increment):** If set to 1, the DMA moves to the next memory address after every byte. If 0, it overwrites the same address.
* **CIRC (Circular Mode):** If set to 1, the CNDTR automatically reloads to its original value when it reaches 0, allowing for continuous data reception.
* **TCIE (Transfer Complete Interrupt Enable):** Enables the hardware to trigger an interrupt when the transfer finishes.

---

## 4. Critical Rules (Things to Remember)

### The Initialization Sequence

We cannot change the configuration while the DMA is active. The sequence must be:

1. Enable DMA Clock (`RCC`).
2. Set Addresses (`CPAR`, `CMAR`).
3. Set Count (`CNDTR`).
4. Configure Mode (`CCR`).
5. **Enable Channel** (`EN` bit in `CCR`) **LAST**.


* **Rule:** We must manually clear this flag in your Interrupt Service Routine (ISR) by writing to the `DMA_IFCR`.
* **Consequence:** If we do not clear it, the interrupt will trigger repeatedly, locking the CPU in an infinite loop.

### The Handshake (DMAR)

Configuring the DMA is not enough. We must also tell the UART to send requests to the DMA.

* This is done by setting the **DMAR** (DMA Receive) bit in `USART_CR3`. Without this, the UART holds the data and never signals the DMA.

---

## 5. Code Explanation

### The Logic Flow

1. **Clock Setup:** The system is boosted to 72MHz to simulate real-world EV performance requirements.
2. **Peripheral Clocks:** Clocks for DMA1, GPIOA, and USART1 are enabled first to prevent bus faults.
3. **DMA Configuration:**
* Source set to `USART1->DR`.
* Destination set to `rx_buffer`.
* Circular mode enabled so the buffer acts as a "ring" (always accepting new data).
* Interrupt enabled (NVIC) to wake the CPU when the buffer is full.

4. **UART Configuration:** Set to 115200 baud. The `DMAR` bit is set to link the UART RX event to the DMA controller.
5. **Main Loop:** The `while(1)` loop is intentionally empty. The data transfer happens entirely in hardware.
6. **Interrupt Handler:** The function `DMA1_Channel5_IRQHandler` runs only when 16 bytes have been received. It clears the completion flag to prepare for the next interrupt.



## Edit: NOTE

1. The ISR Name is FIXED: Defined in the startup file (e.g., DMA1_Channel5_IRQHandler). We cannot rename this.

2. The Hardware Route is FIXED: USART1_RX is hard-wired to DMA1 Channel 5. We cannot swap this to Channel 4. We must check the DMA Request Mapping Table in the Reference Manual to know which channel to use.

3. The Trigger Condition is CONFIGURABLE: Even though the channel is fixed, We can choose when to interrupt.

Do we want an interrupt when it's completely done? (Set TCIE)

Do we want an interrupt when it's half done? (Set HTIE)
