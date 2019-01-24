/*
===============================================================================
 Name        : hw1.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#define FIO0DIR (*(volatile unsigned int *)0x2009c000)
#define FIO0PIN (*(volatile unsigned int *)0x2009c014)

// TODO: insert other definitions and declarations here
void wait_ticks(int count)
{
	volatile int ticks = count;
	while (ticks>0)
		ticks--;
}

int main(void)
{

    // TODO: insert code here
	FIO0DIR |= (1<<22);		// configure LED's port bit as an output

    while(1) {
    	FIO0PIN |= (1<<22);		// turn on LED
    	wait_ticks(8100);
    	FIO0PIN &= ~(1<<22);	// turn off LED
    	wait_ticks(32500);
    }

    return 0;
}
