/*
===============================================================================
 Name        : HW5.c
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
#include <math.h>

// TODO: insert other include files here
#define DACR (*(volatile unsigned int *) 0x4008C000)		// D/A Converter Register -- This read/write register includes the digital value to be converted to analog
#define PINSEL1 (*(volatile unsigned int *) 0x4002C004)		// Pin Function Select Register 1 --  controls the functions of the upper half of Port 0
#define PCONP (*(volatile unsigned int *) 0x400FC0C4)		// Power Control for Peripherals register -- r allows turning off selected peripheral functions for the purpose of
															//     saving power.
#define AD0CR (*(volatile unsigned int *) 0x40034000)		// A/D Control Register
#define AD0GDR (*(volatile unsigned int *)  0x40034004)		// A/D Global Data Register --  holds the result of the most recent A/D conversion that has
															// 	   completed, and also includes copies of the status flags that go with that conversion.
#define PCLKSEL0 (*(volatile unsigned int *) 0x400FC1A8)	// Peripheral Clock Selection registers 0 --  controls the rate of the clock signal
															//     that will be supplied
#define PINMODE1  (*(volatile unsigned int *) 0x4002C044)	// Pin Mode select register 0 -- controls pull-up/pull-down resistor configuration for Port 0 pins 0 to 15
#define AD0STAT (*(volatile unsigned int *)  0x40034030)	// A/D Status register -- allows checking the status of all A/D channels simultaneously.
#define PCLKSEL0 (*(volatile unsigned int *)  0x400FC1A8)	// Peripheral Clock Selection register 0

// struct for FIO object
struct GPFOstruct {

	volatile unsigned int FIODIR;
	int junk[3];
	volatile unsigned int FIOMASK;
	volatile unsigned int FIOPIN;
	volatile unsigned int FIOSET;
	volatile unsigned int FIOCLR;

};
#define FIO ((struct GPFOstruct *)0x2009c000)				// FIO for output

// TODO: insert other definitions and declarations here
// helper functions
void wait_ticks();											// for the wait
void highFreq(int ampTemp, int state);						// frequency generator function for the button pressed state (generates high freq.)
void lowFreq(int ampTemp, int state);						// frequency generator function for the button pressed state (generates low freq.)
_Bool doorbellPressed();									// function for checking the state of the button being pressed.

int wave[256];												// waves for the audio
int result;													// result variable that holds the value of the ambient light


/********************        main    *****************************/
int main(void)
{

	int amp = 511; 					// half a sec <--- 1023 = 1sec
	int bellStarted = 0;			// bell state variable that starts as 0

	// rest of the bit location are set to 1 (NOT pin)
	FIO[2].FIODIR &= ~(1<<8);		// switch set up as input
	FIO[0].FIODIR |= (1 << 25);		// LED set up as output

	// FOR AUDIO PINSEL
	// 21:20   function 10  P0.26
	PINSEL1 |= (1<<21);		// GPIO Port 0.26 has been selected for AOUT with when function 10
	PINSEL1 &= ~(1<<20);	// audio set up as output

	// for loop for initializing the values of the waves in the array using sine waves function
	for (int i = 0; i < 256; i++)
	{
		wave[i] = (511 * sin(3.14159*2*i/256));		// this produces the sinewaves.
	}

	// For LED check
	PCONP|=(1<<12);
	AD0CR=0;
	PCONP &=~(1<<12);

//	1. Power: In the PCONP register (Table 46), set the PCADC bit.
//	Remark: On reset, the ADC is disabled. To enable the ADC, first set the PCADC bit,
//	and then enable the ADC in the AD0CR register (bit PDN Table 532). To disable the
//	ADC, first clear the PDN bit, and then clear the PCADC bit.
	PCONP |= (1<<12);
	AD0CR |= (1<<21);

	AD0CR = (1 << 0) | (1 << 21);

//	2. Clock: In the PCKLSEL0 register (Table 40), select PCLK_ADC. To scale the clock for
//	the ADC, see bits CLKDIV in Table 532.
//  25:24 PCLK_ADC Peripheral clock selection for ADC. 00
	PCLKSEL0 |= (1<<25);
	PCLKSEL0 &= ~(1<<24);

//	3. Pins: Enable ADC0 pins through PINSEL registers. Select the pin modes for the port
//	pins with ADC0 functions through the PINMODE registers (Section 8.5).
//	AD0CR = 0;  // reset  ADC
//	PCONP &= ~(1<<12);
//	PCONP |= (1<<12);
//	AD0CR = (1<<21);

	PINSEL1 |=(1<<14);
	PINSEL1 &=~(1<<15);
	PINMODE1 &= ~(1<<15);
	PINMODE1 |= (1<<14);

	// ACTUATE
	while(1)
	{
		// audio
		if (doorbellPressed())				// if the button is pressed
		{
			bellStarted = 1;    			// sets the state as presssed
			highFreq(amp, 0);				// frequncy generated using the amp value we declared globally. we also pass in the state of bell
			while(doorbellPressed()) {} 	// waits for the button to be released
		}
		else if ( (doorbellPressed() == 0) && (bellStarted == 1) )	// if the button is released and when the button state is 1
		{															// button state of 1 had to be added here so that unnecessary low frequency
																	// avoided on at the start can be
			lowFreq(amp, 1);	// low freq function called with the amp value and on 1

			while(doorbellPressed() == 0)	// while switch released
			{
				// check for ambient light so that it would turn off when high light and turn on when low light
				AD0CR |= (1 << 24);
				while (((AD0GDR>>31)&1)==0) {}
				result = (AD0GDR >> 4) & 0xFFF;
				AD0CR &=~(1<<24);

				if (result < 1570)					// <-- value selected for the threshold.
				{
					FIO[0].FIOPIN |= (1 << 25);		// TURN ON
				}
				else
				{
					FIO[0].FIOPIN &= ~(1 << 25);	// TURN OFF
				}
			}
		}
		else
		{
			// check for ambient light so that it would turn off when high light and turn on when low light
			AD0CR |= (1 << 24);
			while (((AD0GDR>>31)&1)==0) {}
			result = (AD0GDR >> 4) & 0xFFF;			// THE SAME AS ABOVE
			AD0CR &=~(1<<24);

			if (result < 1530)
			{
				FIO[0].FIOPIN |= (1 << 25);
			}
			else
			{
				FIO[0].FIOPIN &= ~(1 << 25);
			}

		}
	}
    return 0;
}				// end of the main loop

// wait function given in lab one
void wait_ticks(int count)
{
	volatile int ticks = count;
	while (ticks>0)
		ticks--;
}

// a function of type _Bool that  keeps the state of the switch being presssed or not.
_Bool doorbellPressed()
{
	// looking to see if the port 2 and bit 8 is pressed and set to 0 (this would normally be 1
	// until the switch is pressed since the switch is set as active low mode
	return (((FIO[2].FIOPIN >> 8) & 1) == 0);
}

// a function that generates high frequency using amp and state as paramters
// amp is set as 511 and state will be passed as a 0 or a 1.
void highFreq(int ampTemp, int state)
{
	int ampTemp2 = ampTemp;
	while( (ampTemp2 > 0) && (((FIO[2].FIOPIN >> 8) & 1) == state) )
	{
		for(int i = 0; i < 256; i++)
		{
			DACR = ((512 + ((wave[i]*ampTemp2) >> 9))) << 6;
			//wait_ticks(1000);
			//i= (i+10) & 255;   /// (i+1) % 256;
			wait_ticks(4);
			i= (i+4) & 255;
			// if (amp < 0) continue;
		}
		ampTemp2--; // amp going down once per cycle
	}
}

// same as above but for a low frequency.
void lowFreq(int ampTemp, int state)
{
	int ampTemp2 = ampTemp;
	while( (ampTemp2 > 0) && (((FIO[2].FIOPIN >> 8) & 1) == state) )
	{
		for(int i = 0; i < 256; i++)
		{
			DACR = ((512 + ((wave[i]*ampTemp2) >> 9))) << 6;
			//wait_ticks(1000);
			//i= (i+10) & 255;   /// (i+1) % 256;
			wait_ticks(7);
			i= (i+4) & 255;
			// if (amp < 0) continue;
		}
		ampTemp2--; // amp going down once per cycle
	}
}
