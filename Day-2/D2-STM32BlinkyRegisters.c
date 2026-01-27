// #include <stm32f1xx.h>
#include <stdint.h>
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
    *(volatile uint32_t *)0x40011004 |= (0b10<<20); //CRH Mode to output
    *(volatile uint32_t *)0x4001100C |= (1<<13); //Turn off LED initially (Active Low)
    while(1){
        *(volatile uint32_t *)0x4001100C ^= (1<<13); //Toggle PC13
        for(volatile uint32_t i=0;i<100000;i++); //Delay
    }
    return 0;
}