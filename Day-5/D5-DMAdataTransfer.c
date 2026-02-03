#include <stdint.h>
volatile uint8_t rx_buffer[16]; 

int main() {
    // Enable HSE
    *(volatile uint32_t *)0x40021000 |= (1<<16);
    while( !(*(volatile uint32_t *)0x40021000 & (1<<17)) );
    
    // Flash Latency (2 wait states)
    *(volatile uint32_t *)0x40022000 |= 0x12; 
    
    // PLL (HSE x 9)
    *(volatile uint32_t *)0x40021004 |= (0b0111<<18); 
    *(volatile uint32_t *)0x40021000 |= (1<<24); // Enable PLL
    while( !(*(volatile uint32_t *)0x40021000 & (1<<25)) );
    
    // Switch System Clock to PLL
    *(volatile uint32_t *)0x40021004 |= (2<<0);
    while( (*(volatile uint32_t *)0x40021004 & (3<<2)) != (2<<2) );

    // --- 2. PERIPHERAL CLOCK ENABLE ---
    // CRITICAL: Turn on clocks BEFORE configuring pins or UART registers
    // DMA1 (AHB), GPIOA (APB2), USART1 (APB2), AFIO (APB2)
    *(volatile uint32_t *)0x40021014 |= (1<<0); // DMA1
    *(volatile uint32_t *)0x40021018 |= (1<<14) | (1<<2) | (1<<0); // USART1, GPIOA, AFIO

    // --- 3. GPIO CONFIGURATION ---
    // PA9 (TX) -> AF Push-Pull (0xB), PA10 (RX) -> Input Floating (0x4)
    *(volatile uint32_t *)0x40010804 &= ~((0xF << 4) | (0xF << 8)); // Clear
    *(volatile uint32_t *)0x40010804 |= ((0xB << 4) | (0x4 << 8));  // Set

    // --- 4. DMA CONFIGURATION (Channel 5 for USART1_RX) ---
    // A. Set Peripheral Address (USART1_DR: 0x40013804)
    *(volatile uint32_t *)0x40020060 = 0x40013804;
    
    // B. Set Memory Address
    *(volatile uint32_t *)0x40020064 = (uint32_t)rx_buffer;
    
    // C. Set Count (16 bytes)
    *(volatile uint32_t *)0x4002005C = 16;
    
    // D. Configure Channel (MINC | CIRC | TCIE)
    // Memory Increment (Bit 7), Circular Mode (Bit 5), Transfer Complete Interrupt (Bit 1)
    // NOTE: We do NOT set Bit 0 (EN) yet!
    *(volatile uint32_t *)0x40020058 = (1<<7) | (1<<5) | (1<<1);

    // E. Enable DMA Interrupt in NVIC
    // IRQ #15 (DMA1_Channel5) is in NVIC_ISER0
    *(volatile uint32_t *)0xE000E100 |= (1<<15);

    // F. Enable DMA Channel
    *(volatile uint32_t *)0x40020058 |= (1<<0);

    // --- 5. UART CONFIGURATION ---
    // Baud Rate 115200 (72MHz / 115200 = 625 = 0x271)
    *(volatile uint32_t *)0x40013808 = 0x271;
    
    // Enable DMA Mode for Receiver (DMAR bit)
    *(volatile uint32_t *)0x40013814 |= (1<<6);
    
    // Enable UART, TX, RX
    *(volatile uint32_t *)0x4001380C |= (1<<13) | (1<<3) | (1<<2);

    // --- 6. MAIN LOOP ---
    while(1) {
        // Do nothing. The DMA is working in the background.
    }
}

// --- 7. INTERRUPT SERVICE ROUTINE ---
// This function is called automatically by hardware when 16 bytes are received
void DMA1_Channel5_IRQHandler(void) {
    // Check if Transfer Complete Flag (TCIF5) is set (Bit 17 of DMA_ISR)
    if (*(volatile uint32_t *)0x40020000 & (1<<17)) {
        
        // CLEAR the flag (Write 1 to Bit 17 of DMA_IFCR)
        // If we don't do this, the interrupt fires infinitely.
        *(volatile uint32_t *)0x40020004 = (1<<17);
        
    }
}