# Day 9: CAN Physical Layer, Error States, and CAN FD (Flexible-Data rate)


**Focus:** Understanding how the CAN signal looks on a scope, CAN error handling, and CAN FD.

## 1. Oscilloscope Readings: The Physical Layer

The "logic" is inverted compared to standard electronics:

* **Recessive (Logic 1):** This is the idle state. Both CAN_H and CAN_L sit at **2.5V**. The differential voltage is roughly **0V**.
* **Dominant (Logic 0):** This is the active state. CAN_H is driven up to **3.5V**, and CAN_L is pulled down to **1.5V**. The differential voltage is **2.0V**.

| State | Logic Level | CAN_H Voltage | CAN_L Voltage | Differential (Vdiff) |
| --- | --- | --- | --- | --- |
| **Recessive** | 1 | 2.5V | 2.5V | ~0V |
| **Dominant** | 0 | 3.5V | 1.5V | ~2V |

If we measure 2.5V on both lines constantly, the bus is idle. If we see a clean square wave on one but noise on the other, the transceiver or termination is likely faulty.

## 2. CAN Error Management:

Every node has two counters:

* **TEC (Transmit Error Counter):** Increments by 8 when this node fails to transmit (e.g., no ACK). Decrements by 1 on success.
* **REC (Receive Error Counter):** Increments by 1 when this node detects an error on the bus. Decrements by 1 on success.

Nodes go to different states based on these counter values:

### State 1: Error Active

* **Trigger:** TEC < 128 and REC < 128.
* **Behaviour:** The node is healthy. It participates in bus traffic and can destroy bad frames by sending an **Active Error Flag** (6 Dominant bits).

### State 2: Error Passive

* **Trigger:** TEC > 127 or REC > 127.
* **Behaviour:** The node is "suspect." It can still send data, but if it detects an error, it must send a **Passive Error Flag** (6 Recessive bits), which doesn't destroy the traffic for others. It also has to wait longer (Suspend Transmission Time) before sending again.

### State 3: Bus Off

* **Trigger:** TEC > 255.
* **Behaviour:** The node effectively disconnects itself from the bus to protect the network. It stops transmitting entirely.
* **Recovery:** To exit this state, the node must observe 128 occurrences of 11 consecutive Recessive bits (monitor the bus for a long idle period) or be manually reset via software.

## 3. CAN FD (Flexible Data-Rate)

CAN FD was introduced for EVs and ADAS. It has faster and longer transmission.

**Key Differences:**

1. **Dual Clock:** The **Arbitration Phase** (ID and ACK) stays at the standard speed (e.g., 500 kbps), but the **Data Phase** switches to a higher speed (up to 8 Mbps).
2. **Payload:** Maximum data length increased from **8 bytes** to **64 bytes**.
3. **New Control Bits:**
* **FDF (FD Format):** Distinguishes FD frames from Classical ones.
* **BRS (Bit Rate Switch):** Determines if the speed jumps during the data phase.
* **ESI (Error State Indicator):** Tells other nodes if the sender is Error Active or Passive.



### CAN FD Frame Format


```text
| SOF | Arbitration Field |            Control Field (FD)              |    Data Field   | CRC Field | ACK | EOF |
| (1) | (11 or 29 bits)   | FDF(1) | res(1) | BRS(1) | ESI(1) | DLC(4) | (0-64 Bytes)    | (17/21)   | (2) | (7) |

```

* **FDF:** Recessive (1) indicates CAN FD frame.
* **BRS:** Recessive (1) enables the higher bit rate for the Data phase.
* **ESI:** Dominant (0) means Error Active; Recessive (1) means Error Passive.
