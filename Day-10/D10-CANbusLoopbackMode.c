#include "stm32f1xx.h"

// MISRA Compliant
#define CAN_500KBPS_PRESCALER   3U  // Value 3 means Prescaler = 4
#define CAN_500KBPS_TS1        13U  // Value 13 means 14 Time Quanta
#define CAN_500KBPS_TS2         2U  // Value 2 means 3 Time Quanta

// Functions
void CAN1_Init_Loopback(void);
void CAN1_Tx(uint32_t std_id, uint8_t *data, uint8_t len);
void CAN1_Rx_Poll(void);

// For verification
uint8_t RxData[8];
uint32_t RxID;

void CAN1_Init_Loopback(void) {
    // 1. Enable Clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;  // Enable CAN1 clock

    // 2. Configure PA11 (RX) and PA12 (TX)
    // Mode: Alternate Function (10 in binary)
    GPIOA->MODER &= ~(GPIO_MODER_MODE11 | GPIO_MODER_MODE12); 
    GPIOA->MODER |=  (GPIO_MODER_MODE11_1 | GPIO_MODER_MODE12_1);
    
    // Alternate Function: AF9 (CAN1) for F4 Series
    // AFR[1] controls pins 8-15. PA11 is at pos 12 (4 bits per pin), PA12 at pos 16
    GPIOA->AFR[1] &= ~((0xFU << 12) | (0xFU << 16)); // Clear
    GPIOA->AFR[1] |=  ((9U << 12)   | (9U << 16));   // Set AF9

    // 3. Enter Initialization Mode
    CAN1->MCR |= CAN_MCR_INRQ; 
    while((CAN1->MSR & CAN_MSR_INAK) == 0U); // Wait for hardware acknowledgement

    // 4. Exit Sleep Mode
    CAN1->MCR &= ~CAN_MCR_SLEEP;
    while((CAN1->MSR & CAN_MSR_SLAK) != 0U); // Wait for wake up

    // 5. Configure Bit Timing & Loopback
    CAN1->BTR = CAN_BTR_LBKM |          // Loopback Mode
                (0U << 24)   |          
                (CAN_500KBPS_TS2 << 20) | 
                (CAN_500KBPS_TS1 << 16) | 
                (CAN_500KBPS_PRESCALER << 0);

    // 6. Filter Configuration (Accept ALL)
    CAN1->FMR |= CAN_FMR_FINIT;     // Enter Filter Init Mode
    CAN1->FM1R &= ~1U;              // Bank 0: Mask Mode
    CAN1->FS1R |= 1U;               // Bank 0: Single 32-bit scale
    
    // Filter Registers (Bank 0)
    CAN1->sFilterRegister[0].FR1 = 0U; // ID Register (Everything is 0)
    CAN1->sFilterRegister[0].FR2 = 0U; // Mask Register (0 means "Don't Care")

    CAN1->FA1R |= 1U;               // Activate Filter Bank 0
    CAN1->FMR &= ~CAN_FMR_FINIT;    // Exit Filter Init Mode

    // 7. Leave Initialization Mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while((CAN1->MSR & CAN_MSR_INAK) != 0U); // Wait for Normal Mode
}

void CAN1_Tx(uint32_t std_id, uint8_t *data, uint8_t len) {
    // 1. Check if Mailbox 0 is empty (TME0 bit)
    if ((CAN1->TSR & CAN_TSR_TME0) == CAN_TSR_TME0) {
        
        // 2. Clear previous data
        CAN1->sTxMailBox[0].TIR = 0U;
        CAN1->sTxMailBox[0].TDTR = 0U;
        CAN1->sTxMailBox[0].TDLR = 0U;
        CAN1->sTxMailBox[0].TDHR = 0U;

        // 3. Set Standard Identifier
        CAN1->sTxMailBox[0].TIR |= (std_id << 21);
        
        // 4. Set Data Length (DLC)
        CAN1->sTxMailBox[0].TDTR |= (len & 0xFU);

        // 5. Load Data
        CAN1->sTxMailBox[0].TDLR = (uint32_t)data[0] | 
                                   ((uint32_t)data[1] << 8) | 
                                   ((uint32_t)data[2] << 16) | 
                                   ((uint32_t)data[3] << 24);
                                   
        CAN1->sTxMailBox[0].TDHR = (uint32_t)data[4] | 
                                   ((uint32_t)data[5] << 8) | 
                                   ((uint32_t)data[6] << 16) | 
                                   ((uint32_t)data[7] << 24);

        // 6. Request Transmission (TXRQ)
        CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;
    }
}

void CAN1_Rx_Poll(void) {
    // 1. Check FIFO 0 Message Pending (FMP0 bits in RF0R)
    if ((CAN1->RF0R & CAN_RF0R_FMP0) != 0U) {
        
        // 2. Read ID (Shift Right by 21 to get Standard ID)
        RxID = (CAN1->sFifoMailBox[0].RIR >> 21) & 0x7FFU;

        // 3. Read Data Length
        uint8_t len = (CAN1->sFifoMailBox[0].RDTR) & 0xFU;

        // 4. Read Data Bytes
        uint32_t dataLow = CAN1->sFifoMailBox[0].RDLR;
        uint32_t dataHigh = CAN1->sFifoMailBox[0].RDHR;

        RxData[0] = (uint8_t)(dataLow & 0xFF);
        RxData[1] = (uint8_t)((dataLow >> 8) & 0xFF);
        RxData[2] = (uint8_t)((dataLow >> 16) & 0xFF);
        RxData[3] = (uint8_t)((dataLow >> 24) & 0xFF);
        RxData[4] = (uint8_t)(dataHigh & 0xFF);
        RxData[5] = (uint8_t)((dataHigh >> 8) & 0xFF);
        RxData[6] = (uint8_t)((dataHigh >> 16) & 0xFF);
        RxData[7] = (uint8_t)((dataHigh >> 24) & 0xFF);

        // 5. Release FIFO to prevent overflow
        CAN1->RF0R |= CAN_RF0R_RFOM0;
    }
}

int main(void) {
    CAN1_Init_Loopback();

    uint8_t tx_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44};

    while (1) {
        // Transmit ID 0x123 with 8 bytes
        CAN1_Tx(0x123, tx_data, 8);
        
        // Simple delay to prevent flooding
        for(int i = 0; i < 500000; i++); 

        // Poll for receiving
        CAN1_Rx_Poll();
        
    }
}
