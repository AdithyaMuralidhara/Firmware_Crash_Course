# Day 10: Bare-Metal CAN Bus Implementation (bxCAN)


##  Bit Timing

In CAN, a single "bit" is not a simple high/low pulse. It is a synchronized time window divided into smaller atomic units called **Time Quanta (Tq)**. This allows the hardware to resynchronize and compensate for clock drift between nodes.



### 1. The Formula
To achieve a specific Baud Rate (e.g., 500 kbps), the APB1 Peripheral Clock ($f_{PCLK}$) is divided by a Prescaler and the number of Time Quanta per bit.

$$BaudRate = \frac{f_{PCLK}}{Prescaler \times (1 + TS1 + TS2)}$$

### 2. The Calculation (Example: 500 kbps @ 36 MHz)
* **Target:** 500,000 bps
* **Clock:** 36,000,000 Hz
* **Required Divisor:** $36,000,000 / 500,000 = 72$

To achieve a divisor of 72, the following values were selected to maintain a Sample Point of ~87.5% (Automotive Standard):
* **Prescaler:** 4
* **Total Tq:** 18 ($1 \text{ (Sync)} + 14 \text{ (TS1)} + 3 \text{ (TS2)}$)
* **Check:** $4 \times 18 = 72$.

### 3. Note:
The STM32 registers use 0-based indexing. The value written to the register is always `Value - 1`.

| Parameter | Actual Value | Register Value (Binary) | Register Field |
| :--- | :--- | :--- | :--- |
| **Prescaler** | 4 | **3** (`0000000011`) | `CAN_BTR[9:0]` (BRP) |
| **Time Seg 1** | 14 Tq | **13** (`1101`) | `CAN_BTR[19:16]` (TS1) |
| **Time Seg 2** | 3 Tq | **2** (`010`) | `CAN_BTR[22:20]` (TS2) |

---

## Algorithm

### 1. Initialization
Before the CAN bus can operate, it must be configured, and filters must be set.

1.  **Clock Enable:** Enable `RCC_APB1ENR_CAN1EN` and GPIO clocks.
2.  **GPIO Config:** Set RX (PA11) and TX (PA12) to **Alternate Function Mode** (AF9 for STM32F4).
3.  **Enter Init Mode:**
    * Set `INRQ` (Initialization Request) bit in `CAN_MCR`.
    * **Wait** until `INAK` (Init Acknowledge) bit in `CAN_MSR` becomes `1`.
4.  **Exit Sleep Mode:**
    * Clear `SLEEP` bit in `CAN_MCR`.
    * **Wait** until `SLAK` (Sleep Acknowledge) bit in `CAN_MSR` becomes `0`.
5.  **Timing Configuration (BTR):**
    * Set `LBKM` (Loopback Mode) bit for testing.
    * Write calculated values for `SJW`, `TS1`, `TS2`, and `BRP`.
6.  **Filter Configuration (Accept All):**
    * Enter Filter Init Mode (`FINIT` bit in `CAN_FMR`).
    * Set Mask Register to `0` (Example: `CAN_FilterRegister[0].FR2 = 0`).
    * Activate Filter Bank 0 (`CAN_FA1R`).
    * Exit Filter Init Mode.
7.  **Start the Bus:**
    * Clear `INRQ` bit in `CAN_MCR`.
    * **Wait** until `INAK` becomes `0`. The bus is now live.



### 2. Transmission Algorithm (Tx)
STM32 provides 3 Transmit Mailboxes. Writing to a busy mailbox corrupts data.

1.  **Check Status:** Verify `TME0` (Transmit Mailbox Empty) bit in `CAN_TSR`.
2.  **Set Identifier:** Write Standard ID (11-bit) to `CAN_TI0R` (Bits 31:21).
3.  **Set DLC:** Write data length (e.g., 8 bytes) to `CAN_TDT0R` (Bits 3:0).
4.  **Load Data:** Write payload to `CAN_TDL0R` (Low 4 bytes) and `CAN_TDH0R` (High 4 bytes).
5.  **Request Transmission:** Set `TXRQ` bit in `CAN_TI0R`.
6.  **Verify:** Check `TXOK0` bit in `CAN_TSR` to confirm successful transmission.

### 3. Receiving Algorithm (Rx)
1.  **Poll FIFO:** Check `FMP0` (FIFO Message Pending) bits in `CAN_RF0R`.
    * If `FMP0 != 00`, a message is waiting.
2.  **Read Identifier:** Read `CAN_RI0R` (Shift right by 21 for Standard ID).
3.  **Read Data:** Extract payload from `CAN_RDL0R` and `CAN_RDH0R`.
4.  **Release FIFO:** Set `RFOM0` bit in `CAN_RF0R`.
    * *Critical:* If this step is skipped, the FIFO will fill up (3 messages) and lock.

---

## Things to Remember

### The "Accept All" Mask
In CAN hardware, filters work using a **Mask**.
* **Logic:** `(Incoming_ID & Mask) == (Filter_ID & Mask)`
* To accept **everything**, set the Mask to `0`.
* *Why?* Because `Anything & 0` is `0`, and `0 == 0` is always True.

### Loopback Mode (`LBKM`)
* **Function:** Disconnects the RX pin from the external bus and internally connects it to the TX output.
* **Purpose:** Allows the MCU to "hear" its own transmissions without a CAN transceiver or a physical bus.
* **Constraint:** We cannot communicate with other nodes in this mode.

### Operating Modes
1.  **Sleep Mode:** Default state after reset. Low power.
2.  **Initialization Mode:** The *only* mode where `CAN_BTR` (Timing) can be modified.
3.  **Silent Mode:** Listen only. It doesn't transmit anything.
