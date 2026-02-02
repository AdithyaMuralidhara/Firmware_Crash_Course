#include<stdint.h>
#include<stdio.h>

int main(){
    // 1. Enable HSE
    *(volatile uint32_t *)0x40021000 |= (1<<16);
    

    // &= Changes Register but & checks the register. Added '!' to wait WHILE the bit is 0 (Not Ready).
    while( !(*(volatile uint32_t *)0x40021000 & (1<<17)) ); //Wait for HSE Ready

    // At 72MHz, Flash is too slow. We must add 2 wait states (Bit 1) and enable prefetch (Bit 4).
    *(volatile uint32_t *)0x40022000 |= 0x12; //0b00010010 where Bit 4 = Prefetch, Bits 2:0 = 010 (2 wait states)

    // Configure PLL. 
    // We need x9 (72MHz). 0b0111 (7) + 2 = 9.
    *(volatile uint32_t *)0x40021004 |= (0b0111<<18);

    // Enable PLL
    *(volatile uint32_t *)0x40021000 |= (1<<24);

    //Wait for PLL Ready
    while( !(*(volatile uint32_t *)0x40021000 & (1<<25)) );

    // Select PLL as System Clock.
    // We need to write binary '10' to bits 1:0. 
    // '2<<0' sets Bit 1 to 1.
    *(volatile uint32_t *)0x40021004 |= (2<<0);

    // NEW LINE: Wait for System Clock Switch (SWS) to confirm PLL is used.
    // We wait until Bits 3:2 read as '10'.
    while( (*(volatile uint32_t *)0x40021004 & (3<<2)) != (2<<2) );

    // Enable Clock for USART1 (Bit 14), GPIOA (Bit 2), AFIO (Bit 0)
    *(volatile uint32_t *)0x40021018 |= (1<<14);
    *(volatile uint32_t *)0x40021018 |= (1<<0) | (1<<2);

    // Removed: ODR/IDR writes (0x4001080C, 0x40010808) are not needed for Alternate Function setup.

    // 1. Clear the fields for Pin 9 and Pin 10 (Bits 4 through 11)
    *(volatile uint32_t *)0x40010804 &= ~((0xF << 4) | (0xF << 8));
    
    // 2. Set Pin 9 to 0xB and Pin 10 to 0x4 as 0xB = AF Push-Pull, 0x4 = Input Floating
    *(volatile uint32_t *)0x40010804 |= ((0xB << 4) | (0x4 << 8));

    // USART CR1: UE (Bit 13), TE (Bit 3), RE (Bit 2) as UE-> USART Enable, TE-> Transmitter Enable, RE-> Receiver Enable
    *(volatile uint32_t *)0x4001380C |= (1<<13) | (1<<3) | (1<<2);
    
    // BRR: 0x271 (Mantissa 39, Fraction 1) -> 115200 Baud @ 72MHz
    *(volatile uint32_t *)0x40013808 |= (0x27<<4);
    *(volatile uint32_t *)0x40013808 |= (0x1<<0);

    while(1) {
        // 1. Wait for a character to arrive (RXNE bit 5)
        while(!(*(volatile uint32_t *)0x40013800 & (1 << 5))); 
        
        // 2. Read the data (this also clears the RXNE flag)
        uint8_t data = *(volatile uint32_t *)0x40013804;

        // 3. Wait for the transmitter to be ready (TXE bit 7)
        while(!(*(volatile uint32_t *)0x40013800 & (1 << 7)));

        // 4. Send the data back (Echo)
        *(volatile uint32_t *)0x40013804 = data;
    }
  
    return 0;
}