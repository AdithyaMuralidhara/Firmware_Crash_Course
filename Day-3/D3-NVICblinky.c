// #include <stm32f1xx.h>
#include <stdint.h>
void EXTI0_IRQHandler(void){
    if(*(volatile uint32_t *)0x40010414 & (1<<0)){
        *(volatile uint32_t *)0x4001100C ^= (1<<13); //Toggle LED
        *(volatile uint32_t *)0x40010414 |= (1<<0); // Clear pending bit (1 to clear)
    }
}
int main(){
    *(volatile uint32_t *)0x40021000 |= (1<<16); // Enable HSE clock
    while(((*(volatile uint32_t *)0x40021000) & (1<<17)) == 0){
        for(volatile uint32_t i=0;i<10000;i++); // Wait for HSE to be ready
    }
    *(volatile uint32_t *)0x40022000 |= 0x02; //Flash_ACK to introduce 2 wait states
    *(volatile uint32_t *)0x40021004 |= (0b0111<<18); // Set PLL to HSE and multiplier to 9
    *(volatile uint32_t *)0x40021004 |= (1<<16); //PLLSRC to HSE
    *(volatile uint32_t *)0x40021000 |= (1<<24); //Enable PLL
    while(((*(volatile uint32_t *)0x40021000) & (1<<25) )== 0){
        for(volatile uint32_t i=0;i<10000;i++); // Wait for PLL to be ready
    }
    *(volatile uint32_t *)0x40021004 |= (0b10<<0); //Set SW to PLL
    while(((*(volatile uint32_t *)0x40021004) & (0b10<<2)) == 0){
        for(volatile uint32_t i=0;i<10000;i++); //Wait for SW to be PLL
    }
    *(volatile uint32_t *)0x40021018 |= (1<<4); //Set IOPCEN
    *(volatile uint32_t *)0x40021018 |= (1<<0); //Set AFIOEN
    *(volatile uint32_t *)0x40021018 |= (1<<2); //Set IOPAEN
    *(volatile uint32_t *)0x40010400 |= (1<<0); //Enable Interrupt Mask for Line 0
    *(volatile uint32_t *)0x4001040C |= (1<<0); //Falling edge trigger for Line 0
    *(volatile uint32_t *)0x40011004 |= (0b10<<20); //CRH Mode to output
    *(volatile uint32_t *)0x4001100C |= (1<<13); //Turn off LED initially (Active Low)
    *(volatile uint32_t *)0x40010800 |= (1<<0); //Set PA0 to input floating
    *(volatile uint32_t *)0x40010800 &= ~(0xF);
    *(volatile uint32_t *)0x40010800 |= (0b0100); //PA0 Mode is Inpiut (00) and CNF is Floating (01) 
    *(volatile uint32_t *)0xE000E100 |= (1<<6); //Enable EXTI0 IRQ in NVIC
    while(1){};
    return 0;
}