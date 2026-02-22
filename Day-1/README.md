# Day 1 
On day 1 of the 30-day bootcamp for firmware development and getting up to speed, we focused on bit manipulation, volatile, static, and const keywords, and code on a FIFO Circular Buffer.


## Bit Manipulation: 

Set Bit: REG |= (1<<5)  //Set 5th Bit only

Clear Bit: REG &= ~(1<<5)  //Clear 5th Bit only

Toggle Bit: REG ^= (1<<5)   //Toggle 5th Bit only


## Keywords

*volatile* - To prevent optimization of the variable. It forces the computer to check the bit whenever called. EX: volatile uint8_t LED_bit;

*static* - To keep the variable active even after the function ends. It also keeps the variable private to that function only and cannot be accessed using the extern keyword.

*const* - To save RAM storage, a variable defined using const is stored in Flash/ROM.


## C Code

1. A struct can be defined outside any function.
2. Inside int main(), buffer members are accessed using  the dot(.) operator, but when the structure is passed as a pointer to a function, it is accessed using the "->" operator.
3. The next position in Circular Queue is found out as, next_pos = (current+1) % size. 
