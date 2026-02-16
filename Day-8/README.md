
# CAN Bus Protocol Notes

## 1. Introduction
The Controller Area Network (CAN) is a robust, multi-master serial communication bus designed for real-time applications. It allows microcontrollers to communicate with each other without a host computer.

**Key Characteristics:**
* **Differential Signaling:** Uses two wires, **CAN High (CAN H)** and **CAN Low (CAN L)**. This provides high immunity to electromagnetic interference.
* **Speed:** Standard CAN supports speeds up to **1 Mbit/s**.
* **Termination:** A **120-ohm resistor** is required at both ends of the bus to suppress signal reflections.

**Logic Levels:**
* **Dominant (Logic 0):** The bus is actively driven. There is a voltage difference between CAN H and CAN L. Dominant bits overwrite recessive bits.
* **Recessive (Logic 1):** The bus is idle (not driven). CAN H and CAN L are at the same voltage level.

---

## 2. CAN Frame Format (Standard CAN 2.0A)

### Frame Structure
The following diagram represents the standard data frame structure.

```text
| SOF |   Identifier   | RTR | IDE | r0 |  DLC  |      DATA      |       CRC       | ACK | EOF | IFS |
| (1) |      (11)      | (1) | (1) |(1) |  (4)  |    (0-64)      |      (15+1)     |(1+1)| (7) | (3) |

```

### Detailed Field Description

| Field | Full Name | Length (Bits) | Description |
| --- | --- | --- | --- |
| **SOF** | Start of Frame | 1 | A single dominant bit (0) marking the start of a message. synchronizes the nodes. |
| **ID** | Identifier | 11 | Sets the message priority. Lower values have higher priority. |
| **RTR** | Remote Transmission Request | 1 | Dominant (0) for Data Frames; Recessive (1) for Remote Frames. |
| **IDE** | Identifier Extension | 1 | Dominant (0) indicates Standard CAN (11-bit ID). |
| **r0** | Reserved Bit | 1 | Reserved for future use (must be dominant). |
| **DLC** | Data Length Code | 4 | Indicates the number of bytes in the Data field (0 to 8 bytes). |
| **Data** | Data Field | 0-64 | The actual payload (0 to 8 bytes). |
| **CRC** | Cyclic Redundancy Check | 15 | Polynomial sequence used for error detection. |
| **CRC Del** | CRC Delimiter | 1 | Must be a recessive bit (1). |
| **ACK** | Acknowledge | 1 | The transmitter sends recessive; the receiver asserts dominant if the message is received correctly. |
| **ACK Del** | ACK Delimiter | 1 | Must be a recessive bit (1). |
| **EOF** | End of Frame | 7 | Seven consecutive recessive bits indicating the end of the message. |
| **IFS** | Inter-Frame Space | 3 | Minimum time gap required between frames for processing. |

---

## 3. Types of Frames

CAN communication uses four specific frame types:

1. **Data Frame:** Carries data from a transmitter to receivers. This is the most common frame type.
2. **Remote Frame:** Transmitted by a node to request the transmission of a Data Frame with the same Identifier.
3. **Error Frame:** Transmitted by any node that detects a bus error. It violates formatting rules to destroy the current traffic and notify other nodes of the error.
4. **Overload Frame:** Used by a receiver to request a delay between data frames if it is not ready to process the next message.

---

## 4. OSI Model Layers

CAN protocol functionality is split across two layers of the OSI model:

### Data Link Layer

Responsible for constructing frames and bus arbitration.

* **LLC (Logical Link Control):** Acceptance filtering, overload notification, and recovery management.
* **MAC (Medium Access Control):** Data encapsulation (framing), arbitration (priority handling), error detection, and serialization/deserialization.

### Physical Layer

Responsible for actual signal transmission.

* **Physical Signaling (PLS):** Bit encoding/decoding and timing.
* **Physical Medium Attachment (PMA):** Driver/Receiver characteristics (CAN H/L voltages).
* **Medium Dependent Interface (MDI):** Connectors and cables.
