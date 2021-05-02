// ADC.h
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/14/21 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
#ifndef ADC_H
#define ADC_H
#include <stdint.h>
#define SAC_NONE 0
#define SAC_2 1
#define SAC_4 2
#define SAC_8 3
#define SAC_16 4
#define SAC_32 5
#define SAC_64 6
// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(uint32_t sac);

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void);
#endif
