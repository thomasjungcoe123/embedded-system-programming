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
// Chime & Melody
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

// RTC
#define SEC (*(volatile unsigned int *)  0x40024020)	// Seconds Counter
#define MIN (*(volatile unsigned int *)  0x40024024)	// Minutes Register
#define HOUR (*(volatile unsigned int *)  0x40024028)	// Hours Register
#define CCR (*(volatile unsigned int *)  0x40024008)	// The clock register is a 4-bit register that controls the operation of the clock divide circuit.
#define ALSEC (*(volatile unsigned int *)  0x40024060)	// Alarm value for Seconds
#define ALMIN (*(volatile unsigned int *)  0x40024064)	// Alarm value for Minutes
#define ALHOUR (*(volatile unsigned int *)  0x40024068)	// Alarm value for Hours
#define ILR (*(volatile unsigned int *)   0x40024000)	// Interrupt Location Register
#define AMR (*(volatile unsigned int *)   0x40024010)	// Alarm Mask Register

// I2C
#define I2C0CONSET (*(volatile unsigned int *) 0x4001C000)	// I2C Control Set Register for master and slave operation between microchip and the other chips such as Tsensor and I/O expander chips
#define I2C0CONCLR (*(volatile unsigned int *) 0x4001C018)	// I2C Control Clear Register for controlling the corresponding bit in the I2C control register, which operates when clears
#define I2C0DAT    (*(volatile unsigned int *) 0x4001C008)	// I2C Data Register for data storage when reading and writing
#define I2C0STAT   (*(volatile unsigned int *) 0x4001C004)	// I2C Status Register for providing detailed status codes that allow software to determine the next action needed.

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
// helper functions for I2C
void initializeI2C(void);			// initializes I2C
void configOutput(void);			// configures I/O expander chip to be output
void I2Cstart(void);				// methods that start I2C
void I2Cstop(void);					// methods that start I2S
int I2Cread(int ack);				// methods for write
void I2Cwrite(int data);			// write to I2C
int readMinSwitchI2C(void);		// read switch from I2C for min
int readHourSwitchI2C(void);		// read switch from I2C for min
int display(int value);				// the same
void writeHourLeft(int value);	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
void writeHourRight(int value);  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)
void writeMinLeft(int value);	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
void writeMinRight(int value);  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)
void writeSecLeft(int value);	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
void writeSecRight(int value);  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)

// int readStartButton(void);			// reads start button for the time.
_Bool startButtomPressed();				// start time button checking function
_Bool setButtomPressed();				// setting time button checking function
_Bool hourButtomPressed();				// hour setting button checking function
_Bool minButtomPressed();				// min setting button checking function

// helper functions for time (RTS)
void initRTC();							// initialize RTC time
void resetRTC();						// reset RTC time
void resetSeconds();					// reset RTC seconds

// helper functions for the alarm
_Bool alarmbuttonPressed();				// checks for alarm button being pressed.
_Bool displayAlarmTime();				// displays time set as an alarm.
void resetAlarm();						// resets the time set as alarm.

// helper functions for the countdown time
_Bool countDownPressed();				// check for countdown buttom being pressed
_Bool countDownDisplay();				// displays the time set for the countdown

// helper functions for the Elapsed Time
_Bool elapsedTimePressed();				// checks for elapsed time button being pressed

// helper functions for chimes
void wait_ticks();											// for the wait
void highFreq(int ampTemp, int state);						// frequency generator function for the button pressed state (generates high freq.)
void lowFreq(int ampTemp, int state);						// frequency generator function for the button pressed state (generates low freq.)
_Bool doorbellPressed();									// function for checking the state of the button being pressed.

void highFreqB3(int ampTemp);								// frequency node for B3 with the parameter values of amplitude predefined with 511.
void highFreqE4(int ampTemp);								// frequency node for E4 with the parameter values of amplitude predefined with 511.
void highFreqD4(int ampTemp);								// frequency node for D4 with the parameter values of amplitude predefined with 511.
void highFreqF4(int ampTemp);								// frequency node for F4 with the parameter values of amplitude predefined with 511.
void highFreqG4(int ampTemp);								// frequency node for G4 with the parameter values of amplitude predefined with 511.

void firstq(int ampTemp);									// sequence of nodes for chime (first part) (this is for each 00's times)
void secq(int ampTemp);										// sequence of nodes for chime (second part)
void thirdq(int ampTemp);									// sequence of nodes for chime (third part)
void last(int ampTemp);										// sequence of nodes for chime (fourth part)
void at15(int ampTemp);										// sequence of nodes for chime for time at 15 min mark
void at30(int ampTemp);										// sequence of nodes for chime for time at 30 min mark
void at45(int ampTemp);										// sequence of nodes for chime for time at 45 min mark
void ding(int ampTemp);										// ding sounds that is suppsed to sound at the end at 00 mark
void soundDatDing();										// helper function that utilizes ding function to sound that ding
void turnLEDOn();											// turns on LED on the displays

// *********************  CONSTANTS   *************************** //
//  *********   chime/melody constants    **********************  //
int amp = 4095;

//  ******************   I2C constants    **********************  //
// address -> 1001(A2)(A1)(A0)
// read = 1 && write = 0;
const int addrExpanHour = 0b0100000;	// selected address of the I/O Expander chip for the hour
const int addrExpanMin =  0b0100100;	// selected address of the I/O Expander chip for the min
const int addrExpanSec =  0b0100110;	// selected address of the I/O Expander chip for the sec


// declaration for the chimes
//int wave[256];											// waves for the audio

int result;													// result variable that holds the value of the ambient light
int alarmState = 0;											// keeps the state of the alarm

int swMIN = 0;												// keeps the state of the min
int swHOUR = 0;												// keeps the state of the hour

// time constants for alarm
volatile unsigned int countDownSEC = 0;						// global variable for countdown Seconds
volatile unsigned int countDownMIN = 0;						// global variable for countdown Minutes
volatile unsigned int countDownHOUR = 0;					// global variable for countdown Hours
volatile unsigned int countDownState = 0;					// global variable for countdown state
volatile unsigned int countDownStateReal = 0;				// global variable for countdown state 2
volatile unsigned int dispRTCState = 1;						// global variable for RTC state
volatile unsigned int dispCountDownState = 0;				// global variable for countdown display state

// count down time constants
volatile unsigned int valueHOUR = 0;
volatile unsigned int valueMIN = 0;
volatile unsigned int valueSEC = 0;
volatile unsigned int sum = 0;
volatile unsigned int timeTotal = 0;
volatile unsigned int countDownReset = 0;
volatile unsigned int value = 0;

volatile unsigned int checkRTCSEC = 0;						// global variable for comparing SEC and SEC++
volatile unsigned int tempRTCSEC = 0;						// global variable for countdown SEC
volatile unsigned int tempRTCMIN = 0;						// global variable for countdown MIN
volatile unsigned int tempRTCHOUR = 0;						// global variable for countdown HOUR


/********************        main    *****************************/
int main(void)
{
	initializeI2C();		// initialize the I2C
	configOutput();			// configurate the I/O as output
	resetAlarm();			// reset Alarm time to 00,00,00

	// ***********************    Switch for I2C 	   *********************** //
	FIO[2].FIODIR &= ~(1<<7);	// switch for the start the time.
	FIO[2].FIODIR &= ~(1<<6);	// switch for the setting the time.
	FIO[2].FIODIR &= ~(1<<5);	// switch for the Hour time.
	FIO[2].FIODIR &= ~(1<<4);	// switch for the MIN time.

	// ***********************    Switch for Alarm 	   *********************** //
	FIO[2].FIODIR &= ~(1<<3);	// switch for the Alarm setting button.
	FIO[2].FIODIR &= ~(1<<2);	// switch for the Alarm displaying button.

	// ***********************    Switch for CountDown 	   *********************** //
	FIO[2].FIODIR &= ~(1<<1);	// switch for the count down time setting.
	FIO[2].FIODIR &= ~(1<<0);	// switch for the count down time display.

	// ***********************    Switch for Elapsed Time 	   *********************** //
	FIO[0].FIODIR &= ~(1 << 11);		// switch for the elapsed time setting
	FIO[0].FIODIR &= ~(1 << 10);		// switch for the elapsed time displaying

	// *********************** decla./init for dime/song   *********************//
	FIO[2].FIODIR &= ~(1<<8);		// switch set up as input for melody
	FIO[0].FIODIR |= (1 << 25);		// set up as output for chime/melody

	// FOR AUDIO PINSEL
	// 21:20   function 10  P0.26
	PINSEL1 |= (1<<21);		// GPIO Port 0.26 has been selected for AOUT with when function 10
	PINSEL1 &= ~(1<<20);	// audio set up as output

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
	PINSEL1 |=(1<<14);
	PINSEL1 &=~(1<<15);
	PINMODE1 &= ~(1<<15);
	PINMODE1 |= (1<<14);

	// Time using RTC
	initRTC();  				// starting time
	startButtomPressed();		// resets timers to 00, 00, 00

	// alarm ILR regiser (Interrupt Location Register)
	ILR |= (1 << 0);
	ILR |= (1 << 1);			// When one, the alarm registers generated an interrupt. Writing a one to this bit location
								// clears the alarm interrupt.

	// AMR register ignoring stuff (// Alarm Mask Register)
	AMR |= (1 << 3);			// When 1, the Day of Month value is not compared for the alarm.
	AMR |= (1 << 4);			// When 1, the Day of Week value is not compared for the alarm.
	AMR |= (1 << 5);			// When 1, the Day of Year value is not compared for the alarm.
	AMR |= (1 << 6);			// When 1, the Month value is not compared for the alarm.
	AMR |= (1 << 7);			// When 1, the Year value is not compared for the alarm.

	checkRTCSEC = SEC;			// For Count Down Time

	// ACTUATE
	while(1)
	{

		while(setButtomPressed())	// if set button is pressed.
		{
			// stays in this function loop until released

			while(setButtomPressed()) { }
		}

		turnLEDOn();				// display


		// ********   Alarm    ************ //
		while (alarmbuttonPressed())
		{
//			stateTime++;

			// stays in this function loop until released
			while (alarmbuttonPressed()) { }

		}

		while (displayAlarmTime() )
		{

			// stays in this function loop until released
			while (displayAlarmTime()) { }
		}

		// alarm melody case
		if( (alarmState == 1) && (MIN == ALMIN) && (HOUR == ALHOUR) && (SEC == ALSEC))
		{
			meHlody(amp);			// sound that alarm
			alarmState=0;			// alarm state set back to default value
		}

		while (countDownPressed())
		{
			// stays in this function loop until released
			while (countDownPressed()) { }
		}

		// countdown
		while( countDownDisplay() )
		{
//			while (sum > timeTotal){ }

			// stays in this function loop until released
			while( countDownDisplay() ) { }
		}

		// countdown meldy case
		if ( (countDownState == 1) && (countDownStateReal == 1) && (valueHOUR == 0) && (valueMIN == 0)  && (valueSEC == 0) )
		{
			meHlody(amp);					// sound that alarm
			countDownState = 0;				// states set back to default after melody sounded
			countDownStateReal = 0;			// states set back to default after melody sounded
		}

		while(elapsedTimePressed())
		{

			// stays in this function loop until released
			while(elapsedTimePressed()) { }
			countDownReset = 0;
		}

		if ( (MIN == 0) && (SEC == 0) )
		{
			firstq(amp);
			secq(amp);
			thirdq(amp);
			last(amp);
			soundDatDing(); // dinggy dangdang
		}
		if ( (MIN == 15) && (SEC == 0) )
		{
			at15(amp);
		}

		if ( (MIN == 30) && (SEC == 0) )
		{
			at30(amp);
		}

		if ( (MIN == 45) && (SEC == 0) )
		{
			at45(amp);
		}

		// audio part
		if (doorbellPressed())				// if the button is pressed
		{
			// D#4 1290; 311~312Hz
			// E4   1220;   330Hz
			// F4   wait_ticks(1152); 350Hz
			// F#4  wait_ticks(1090); 369~370Hz
			// G#4 970; 413~415hZ
			// b3 1635; 247Hz


			//firstq(amp);
			//secq(amp);
			//thirdq(amp);
			//last(amp);
			//wait_ticks(500000);
			//at15(amp);

			meHlody(amp);
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
void highFreqB3(int ampTemp)
{
	int ampTemp2 = ampTemp;

	while( (ampTemp2 > 0))
	{
		for(int i = 0; i < 511; i++)
		{

			// D#4 1290; 311~312Hz
			// E4   1220;   330Hz
			// F4   wait_ticks(1152); 350Hz
			// F#4  wait_ticks(1090); 369~370Hz
			// G#4 970; 413~415hZ
			// b3 1635; 247Hz
			DACR = ampTemp2 << 6;
//			wait_ticks(1635/4);
			wait_ticks(1635/8);
			DACR = 0;
			wait_ticks(1635/8);

			//ampTemp2--;
		}
		ampTemp2 = 0;
	}
}

//---------------------------------------------------------------------------
void highFreqE4(int ampTemp)
{
	int ampTemp2 = ampTemp;
	while( (ampTemp2 > 0))
	{
		for(int i = 0; i < 511; i++)
		{
			// D#4 1290; 311~312Hz
			// E4   1220;   330Hz
			// F4   wait_ticks(1152); 350Hz
			// F#4  wait_ticks(1090); 369~370Hz
			// G#4 970; 413~415hZ
			// b3 1635; 247Hz

			DACR = ampTemp2 << 6;
//			wait_ticks(1220/4);
			wait_ticks(1220/8);
			DACR = 0;
			wait_ticks(1220/8);

			//ampTemp2--;
		}
		ampTemp2 = 0;
	}
}


//---------------------------------------------------------------------------
void highFreqD4(int ampTemp)
{
	int ampTemp2 = ampTemp;
	while( (ampTemp2 > 0))
	{
		for(int i = 0; i < 511; i++)
		{
			// D#4 1290; 311~312Hz
			// E4   1220;   330Hz
			// F4   wait_ticks(1152); 350Hz
			// F#4  wait_ticks(1090); 369~370Hz
			// G#4 970; 413~415hZ
			// b3 1635; 247Hz

			DACR = ampTemp2 << 6;
//			wait_ticks(1290/4);//
			wait_ticks(1290/8);
			DACR = 0;
			wait_ticks(1290/8);
			//ampTemp2--;
		}
		ampTemp2 = 0;
	}
}

//---------------------------------------------------------------------------
void highFreqF4(int ampTemp)
{
	int ampTemp2 = ampTemp;
	while( (ampTemp2 > 0))
	{
		for(int i = 0; i < 511; i++)
		{
			// D#4 1290; 311~312Hz
			// E4   1220;   330Hz
			// F4   wait_ticks(1152); 350Hz
			// F#4  wait_ticks(1090); 369~370Hz
			// G#4 970; 413~415hZ
			// b3 1635; 247Hz

			DACR = ampTemp2 << 6;
//			wait_ticks(1090/4);
			wait_ticks(1090/8);
			DACR = 0;
			wait_ticks(1090/8);
			//ampTemp2--;
		}
		ampTemp2 = 0;
	}
}

//---------------------------------------------------------------------------
void highFreqG4(int ampTemp)
{
	int ampTemp2 = ampTemp;
	while( (ampTemp2 > 0))
	{
		// 256
		for(int i = 0; i < 511; i++)
		{
			// D#4 1290; 311~312Hz
			// E4   1220;   330Hz
			// F4   wait_ticks(1152); 350Hz
			// F#4  wait_ticks(1090); 369~370Hz
			// G#4 970; 413~415hZ
			// b3 1635; 247Hz

			DACR = ampTemp2 << 6;
//			wait_ticks(970/4);
			wait_ticks(970/8);
			DACR = 0;
			wait_ticks(970/8);
			//ampTemp2--;
		}
		ampTemp2 = 0;
	}
}

//---------------------------------------------------------------------------
void firstq(int ampTemp)
{
	int ampTemp2 = ampTemp;
	highFreqE4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqG4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqF4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqB3(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
}
//---------------------------------------------------------------------------
void secq(int ampTemp)
{
	int ampTemp2 = ampTemp;

	highFreqE4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqF4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqG4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqE4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
}

//---------------------------------------------------------------------------
void thirdq(int ampTemp)
{
	int ampTemp2 = ampTemp;

	highFreqG4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqE4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqF4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqB3(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
}

//---------------------------------------------------------------------------
void last(int ampTemp)
{
	int ampTemp2 = ampTemp;
	highFreqB3(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqF4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqG4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqE4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
}

//---------------------------------------------------------------------------
void at15(int ampTemp)
{
	int ampTemp2 = ampTemp;
	highFreqG4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqF4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqE4(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
	highFreqB3(ampTemp2);
	wait_ticks(50000);
	turnLEDOn();
}

//---------------------------------------------------------------------------
void at30(int ampTemp)
{
	int ampTemp2 = ampTemp;
	firstq(ampTemp2);
	turnLEDOn();
	secq(ampTemp2);
	turnLEDOn();
}

//---------------------------------------------------------------------------
void at45(int ampTemp)
{
	int ampTemp2 = ampTemp;

	turnLEDOn();
	thirdq(ampTemp2);
	turnLEDOn();
}

void ding(int ampTemp)
{
	int ampTemp2 = ampTemp;
	highFreqE4(ampTemp2);
	turnLEDOn();
	wait_ticks(500000);
}

void soundDatDing()
{
	switch(HOUR)
	{
		case 0:
			break;
		case 1: ding(amp);
			break;
		case 2: ding(amp); ding(amp);
			break;
		case 3: ding(amp); ding(amp); ding(amp);
			break;
		case 4: ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 5: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 6: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 7: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 8: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 9: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 10: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 11: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 12: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
		 break;
		case 13: ding(amp);
			break;
		case 14: ding(amp); ding(amp);
			break;
		case 15: ding(amp); ding(amp); ding(amp);
			break;
		case 16: ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 17: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 18: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 19: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 20: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 21: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 22: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;
		case 23: ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp); ding(amp);
			break;

		default:
			break;
	}
}

//---------------------------------------------------------------------------
void meHlody(int ampTemp)
{  // This melody is meh
	int ampTemp2 = ampTemp;

	for (int i=0; i<3;i++)
	{
		wait_ticks(1000);
		highFreqB3(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqG4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqG4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqB3(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqF4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqG4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqG4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqF4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqE4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqG4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqG4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
		highFreqE4(ampTemp2);
		turnLEDOn();
		//wait_ticks(5);
	}
}

//---------------------------------------------------------------------------
// function for initializing I2C
void initializeI2C(void)
{

	// duty high and low (cuts our 1MHz into 100kHz) because fout = 1Mhz/ (I2C0SCLL+I2C0SCLH) = 100kHz
//	I2C0SCLL = 0x05;	// 5 low time
//	I2C0SCLH = 0x05;	// 5 high time

	// enabling I2C
	I2C0CONSET = (1<<6);	// I2EN, I2C interface enable.

	// basic config.
	// 1. Power: In the PCONP register (Table 46), set bit PCI2C0/1/2.
	PCONP |= (1<<7);	// enabling PCI2C0 -- The I2C0 interface power/clock control bit of PCONP

	// 2. Clock: In PCLKSEL0 select PCLK_I2C0; in PCLKSEL1 select PCLK_I2C1 or PCLK_I2C2 (see Section 4.7.3).
//	PCLKSEL0 &= ~(1<<0);	// Peripheral clock selection for I2C0. (00)
//	PCLKSEL0 &= ~(1<<1);   	//  00 --> cclk/4

	// 3. Pins: Select I2C0, I2C1, or I2C2 pins through the PINSEL registers. Select the pin modes for the port pins with I2C1 or I2C2 functions through the PINMODE registers
	// (no pull-up, no pull-down resistors) and the PINMODE_OD registers (open drain) (See Section 8.5).
	PINSEL1 |= (1<<22);		// GPIO Port 0.27 has been selected for SDA0 with when function 01
	PINSEL1 &= ~(1<<23);

	PINSEL1 |= (1<<24);		// GPIO Port 0.28 has been selected for SCL0 with when function 01
	PINSEL1 &= ~(1<<25);
}

//---------------------------------------------------------------------------
// function for configuration of the I/O expansion chip, deciding the direction of the data flow.
// basically setting both side of the Expander chip to be output.
void configOutput(void)
{

	// ***************** hour chip conf ****************//
	I2Cstart();
	I2Cwrite(addrExpanHour << 1); // address of expander chip shifted to left by 1. (// read = 1 && write = 0;))
	// input/output enable
	I2Cwrite(0x00);			  // IODIRA set
	// fix which am i using **
	I2Cwrite(0b10000000);	  // write to I/O Expander chip
	I2Cstop();

	I2Cstart();
	I2Cwrite(addrExpanHour << 1);
	I2Cwrite(0x01);				// IODIRB set
	// fix which am i using ***
	I2Cwrite(0b10000000);
	I2Cstop();


	// ***************** min chip conf ****************//
	I2Cstart();
	I2Cwrite(addrExpanMin << 1); // address of expander chip shifted to left by 1. (// read = 1 && write = 0;))
	// input/output enable
	I2Cwrite(0x00);			  // IODIRA set
	// fix which am i using **
	I2Cwrite(0b10000000);	  // write to I/O Expander chip
	I2Cstop();

	I2Cstart();
	I2Cwrite(addrExpanMin << 1);
	I2Cwrite(0x01);				// IODIRB set
	// fix which am i using ***
	I2Cwrite(0b10000000);
	I2Cstop();


	// ***************** sec chip conf ****************//
	I2Cstart();
	I2Cwrite(addrExpanSec << 1); // address of expander chip shifted to left by 1. (// read = 1 && write = 0;))
	// input/output enable
	I2Cwrite(0x00);			  // IODIRA set
	// fix which am i using **
	I2Cwrite(0b10000000);	  // write to I/O Expander chip
	I2Cstop();

	I2Cstart();
	I2Cwrite(addrExpanSec << 1);
	I2Cwrite(0x01);				// IODIRB set
	// fix which am i using ***
	I2Cwrite(0b10000000);
	I2Cstop();


}

// funciton that starts I2C
void I2Cstart(void)
{

	I2C0CONSET = (1 << 3);  // si enabled
	I2C0CONSET = (1 << 5);  // stat enabled
	I2C0CONCLR = (1 << 3);  // si bit cleared

 	while(((I2C0CONSET>>3)&1)==0) {}	// waiting for conset to be cleared by conclr
	I2C0CONCLR = (1 << 5);	// stat bit cleared
}

// function that stops I2C
void I2Cstop(void)
{
	I2C0CONSET = (1<<4);	// STO enabled
	I2C0CONCLR = (1<<3);	// si bit cleared
	while ((I2C0CONSET & (1 << 4)) != 0) {} 	// waiting for STO to be cleared
	//while (((I2C0CONSET >> 4) & 1) == 1) {}
}

//---------------------------------------------------------------------------
// function for reading I2C
int I2Cread(int ack)
{
	if(ack)	// if > 0
	{
		I2C0CONSET = (1<<2);	//	AA enabled --> acknowledge any address when another device is master of the bus, so it can not enter slave mode
	}
	else	// if 0
	{
		I2C0CONCLR = (1<<2);	// AAC -- Assert acknowledge Clear bit.
	}
	I2C0CONCLR = (1 << 3);		// clearing si bit

//	while (((I2C0CONSET >> 3) & 1) == 0) {}
	while((I2C0CONSET & (1 << 3)) == 0) { }		// wait for si bit to be enabled
	return (I2C0DAT & 0xFF);					// return the data ANDed with 0xFF
}

//---------------------------------------------------------------------------
// function for writing to I2C
void I2Cwrite(int data)
{
	I2C0DAT = data;	// ! bit manipulation
	I2C0CONCLR = (1<<3);	// clear SI

	while((I2C0CONSET & (1 << 3)) == 0) { }	// waiting for si bit to be enabled

//	while(((I2C0CONSET >> 3) & 1) == 0) { }

	switch(I2C0STAT & 0xF8)
	{
		case 0x18:
		case 0x28:
		case 0x40:
		case 0x50: return 1;
		default: return 0;
	}
}

//---------------------------------------------------------------------------
int display(int value)
{
	int hexValue;
	if(value==0){hexValue = 0b0000001; }
	if(value==1){hexValue = 0b1001111; }
	if(value==2){hexValue = 0b0010010; }
	if(value==3){hexValue = 0b0000110; }
	if(value==4){hexValue = 0b1001100; }
	if(value==5){hexValue = 0b0100100; }
	if(value==6){hexValue = 0b0100000; }
	if(value==7){hexValue = 0b0001111; }
	if(value==8){hexValue = 0b0000000; }
	if(value==9){hexValue = 0b0001100; }
	return hexValue;
}

//---------------------------------------------------------------------------
void writeHourLeft(int value)	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
{
	I2Cstart();
	I2Cwrite( (addrExpanHour << 1)|0 );	// shift it by 1 and and it with 0 since we are writing
	I2Cwrite(0x13);					// GPIOB  -- left

	int time = display(value / 10);	// display function called with 10's digit value of the temp
	I2Cwrite(time);
	//	I2Cwrite(display(value / 10));
	I2Cstop();

}

//---------------------------------------------------------------------------
void writeHourRight(int value)  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)
{
	I2Cstart();
	I2Cwrite( (addrExpanHour << 1) | 0 );   // the same as the above
	I2Cwrite(0x12);						// GPIOA -- right

	int time = display(value % 10);		// display function called with 1's digit value of the temp
	I2Cwrite(time);
	// I2Cwrite(display(value % 10));
	I2Cstop();
}

//---------------------------------------------------------------------------
void writeMinLeft(int value)	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
{
	I2Cstart();
	I2Cwrite( (addrExpanMin << 1)|0 );	// shift it by 1 and and it with 0 since we are writing
	I2Cwrite(0x13);					// GPIOB  -- left

	int time = display(value / 10);	// display function called with 10's digit value of the temp
	I2Cwrite(time);
	//	I2Cwrite(display(value / 10));
	I2Cstop();

}

//---------------------------------------------------------------------------
void writeMinRight(int value)  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)
{
	I2Cstart();
	I2Cwrite( (addrExpanMin << 1) | 0 );   // the same as the above
	I2Cwrite(0x12);						// GPIOA -- right

	int time = display(value % 10);		// display function called with 1's digit value of the temp
	I2Cwrite(time);
	// I2Cwrite(display(value % 10));
	I2Cstop();
}

//---------------------------------------------------------------------------
void writeSecLeft(int value)	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
{
	I2Cstart();
	I2Cwrite( (addrExpanSec << 1)|0 );	// shift it by 1 and and it with 0 since we are writing
	I2Cwrite(0x13);					// GPIOB  -- left

	int time = display(value / 10);	// display function called with 10's digit value of the temp
	I2Cwrite(time);
	//	I2Cwrite(display(value / 10));
	I2Cstop();

}

//---------------------------------------------------------------------------
void writeSecRight(int value)  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)
{
	I2Cstart();
	I2Cwrite( (addrExpanSec << 1) | 0 );   // the same as the above
	I2Cwrite(0x12);						// GPIOA -- right

	int time = display(value % 10);		// display function called with 1's digit value of the temp
	I2Cwrite(time);
	// I2Cwrite(display(value % 10));
	I2Cstop();
}

//---------------------------------------------------------------------------
_Bool startButtomPressed()
{
	// p2LEDButtomLeft = 1;
	resetRTC();

	// turns on LED that is connected to the pin that has port 2 and pin 7
	return (((FIO[2].FIOPIN >> 7) & 1) == 0);
}

//---------------------------------------------------------------------------
_Bool setButtomPressed()
{

	while((((FIO[2].FIOPIN >> 6) & 1) == 0))
	{

		while ( hourButtomPressed() )
		{
			HOUR++;
			if (HOUR == 24)
			{
				HOUR = 0;
			}
			writeHourLeft(HOUR);
			writeHourRight(HOUR);

			while (hourButtomPressed()) { }
		}

		while (minButtomPressed())
		{
			MIN++;
			if(MIN==60){
				MIN=0;
			}
			writeMinLeft(MIN);
			writeMinRight(MIN);

			while (minButtomPressed()) { }

		}
		resetSeconds();
	}

	return (((FIO[2].FIOPIN >> 6) & 1) == 0);	  // switch for the setting the time.

	/*
	while((((FIO[2].FIOPIN >> 6) & 1) == 0))
	{

		while ( readHourSwitchI2C() == 0 )
		{
			HOUR++;
			if (HOUR == 24)
			{
				HOUR = 0;
			}
			writeHourLeft(HOUR);
			writeHourRight(HOUR);

			while (readHourSwitchI2C() == 0) { }
		}

		while (readMinSwitchI2C() == 0)
		{
			MIN++;
			if(MIN==60){
				MIN=0;
			}
			writeMinLeft(MIN);
			writeMinRight(MIN);

			while (readMinSwitchI2C() == 0) { }

		}
		resetSeconds();
	}

	return (((FIO[2].FIOPIN >> 6) & 1) == 0);	  // switch for the setting the time.
	*/
}

//---------------------------------------------------------------------------
_Bool hourButtomPressed()
{
	// p2LEDButtomLeft = 1;
	// turns on LED that is connected to the pin that has port 2 and pin 7
	return (((FIO[2].FIOPIN >> 5) & 1) == 0);		// switch for the Hour time.
}

//---------------------------------------------------------------------------
_Bool minButtomPressed()
{
	// p2LEDButtomLeft = 1;
	// turns on LED that is connected to the pin that has port 2 and pin 7
	return (((FIO[2].FIOPIN >> 4) & 1) == 0);		// switch for the MIN time.
}

//---------------------------------------------------------------------------
void resetRTC()
{
	SEC = 0;
	MIN = 0;
	HOUR = 0;
}

//---------------------------------------------------------------------------
void resetSeconds()
{
	SEC = 0;
}

//---------------------------------------------------------------------------
void resetAlarm()
{
	ALSEC =0;
	ALMIN = 0;
	ALHOUR = 0;
}

//---------------------------------------------------------------------------
void initRTC()
{
	// safe case
	PCONP |= (1<<9);
	PCONP &= ~(1<<9);
//	1. Power: In the PCONP register (Table 46), set bits PCRTC.
//	Remark: On reset, the RTC is enabled. See Section 27.7 for power saving options.
	PCONP |= (1<<9);	// The RTC power/clock control bit enable.

//	2. Clock: The RTC uses the 1 Hz clock output from the RTC oscillator as the only clock
//	source. The peripheral clock rate for accessing registers is CCLK/8.

//	3. Interrupts: See Section 27.6.1 for RTC interrupt handling. Interrupts are enabled in
//	the NVIC using the appropriate Interrupt Set Enable register. The RTC interrupt can
//	wake up the device from deep-sleep, power-down, or deep power-down modes.
	CCR |= (1<<0);    // The time counters are enabled   //CLKEN --clock enable
}

//---------------------------------------------------------------------------
_Bool alarmbuttonPressed()
{


	while ( (((FIO[2].FIOPIN >> 3) & 1) == 0) )
	{
		// reset alarm each time we set new alarm time
		//resetAlarm();
		alarmState = 1;

		while (hourButtomPressed())
		{
			ALHOUR++;
			if (ALHOUR == 24)
			{
				ALHOUR = 0;
			}

			writeHourLeft(ALHOUR);
			writeHourRight(ALHOUR);

			while (hourButtomPressed()) { }
		}

		while (minButtomPressed())
		{
			ALMIN++;
			if(ALMIN==60){
				ALMIN=0;
			}

			writeMinLeft(ALMIN);
			writeMinRight(ALMIN);

			while (minButtomPressed()) { }
		}
		ALSEC = 0;
	}

	return (((FIO[2].FIOPIN >> 3) & 1) == 0);		// switch for the MIN time.
}

//---------------------------------------------------------------------------
void turnLEDOn()
{
	writeHourLeft(HOUR);		// display 10th digit of the RTC hour
	writeHourRight(HOUR);		// display 1st digit of the RTC hour
	writeMinLeft(MIN);			// display 10th digit of the RTC min
	writeMinRight(MIN);			// display 1st digit of the RTC min
	writeSecLeft(SEC);			// display 10th digit of the RTC sec
	writeSecRight(SEC);			// display 1st digit of the RTC sec
}

//---------------------------------------------------------------------------
_Bool displayAlarmTime()
{
	while((((FIO[2].FIOPIN >> 2) & 1) == 0))
	{
		writeHourLeft(ALHOUR);		// display 10th digit of the alarm hour
		writeHourRight(ALHOUR);		// display 1st digit of the alarm hour
		writeMinLeft(ALMIN);		// display 10th digit of the alarm min
		writeMinRight(ALMIN);		// display 1st digit of the alarm min
		writeSecLeft(ALSEC);		// display 10th digit of the alarm sec
		writeSecRight(ALSEC);		// display 1st digit of the alarm sec
	}

	return (((FIO[2].FIOPIN >> 2) & 1) == 0);		// switch for the alarm time.
}

//---------------------------------------------------------------------------
_Bool countDownPressed()
{

	while((((FIO[2].FIOPIN >> 1) & 1) == 0))
	{

//		countDownReset++;
		//resetRTC();
		resetAlarm();

		while (hourButtomPressed())
		{
//			if (countDownReset > 1)
//			{
//				continue;
//			}

			countDownHOUR++;
			if (countDownHOUR == 24)
			{
				countDownHOUR = 0;
			}

			writeHourLeft(countDownHOUR);
			writeHourRight(countDownHOUR);

			while (hourButtomPressed()) { }
		}

		while (minButtomPressed())
		{
//			if (countDownReset > 1)
//			{
//				continue;
//			}

			countDownMIN++;
			if(countDownMIN==60){
				countDownMIN=0;
			}

			writeMinLeft(countDownMIN);
			writeMinRight(countDownMIN);

			while (minButtomPressed()) { }
		}
		writeSecLeft(countDownSEC);
		writeSecRight(countDownSEC);
	}

	return (((FIO[2].FIOPIN >> 1) & 1) == 0);
}

//---------------------------------------------------------------------------
_Bool countDownDisplay()
{
	/*
	if (SEC > checkRTCSEC)
	{

		tempRTCSEC++;
		if (tempRTCSEC == 60)
		{
			tempRTCSEC = 0;
			tempRTCMIN++;
		}
		if (tempRTCMIN == 60)
		{
			tempRTCMIN = 0;
			tempRTCHOUR++;
		}
	}
	*/

//	tempRTCSEC = SEC;
//	tempRTCMIN = MIN;
//	tempRTCHOUR = HOUR;


	countDownState=0;

	while((((FIO[2].FIOPIN >> 0) & 1) == 0))
	{
		while (countDownPressed())
		{

			while(countDownPressed()) { }
		}

		countDownState=1;
		dispRTCState = 0;
		dispCountDownState = 1;

		sum = countDownHOUR*3600 + countDownMIN*60;

		while (sum > timeTotal)
		{
			timeTotal++;
			wait_ticks(500000);

//			timeTotal = (tempRTCHOUR * 3600 + tempRTCMIN * 60 + tempRTCSEC);
//			timeTotal = HOUR * 3600 + MIN * 60 + ((++SEC) - SEC);

			value = sum - timeTotal;

			valueHOUR = value/3600;  	// hour
			value = value%3600; 		// get the remainder of the value
			valueMIN = value/60;		// min
			value = value%60;  			// get the remainder of the value
			valueSEC = value%60;  		// sec

			if ((valueHOUR == 0) && (valueMIN == 0)  && (valueSEC == 0))
			{
				countDownStateReal=1;
			}

			while((((FIO[2].FIOPIN >> 0) & 1) == 0))
			{
				if (dispRTCState == 0)
				{
					dispRTCState = 1;
				} else {
					dispRTCState = 0;
				}

				if (dispCountDownState == 0)
				{
					dispCountDownState = 1;
				} else {
					dispCountDownState = 0;
				}
				while((((FIO[2].FIOPIN >> 0) & 1) == 0)) { }
			}


			if (dispRTCState == 1)
			{
				turnLEDOn();
			}else {
				writeHourLeft(valueHOUR);
				writeHourRight(valueHOUR);
				writeMinLeft(valueMIN);
				writeMinRight(valueMIN);
				writeSecLeft(valueSEC);
				writeSecRight(valueSEC);
			}


		}

	}
	return (((FIO[2].FIOPIN >> 0) & 1) == 0);
}

_Bool elapsedTimePressed()
{
	while((((FIO[0].FIOPIN >> 11) & 1) == 0))
	{



	}


	return (((FIO[0].FIOPIN >> 11) & 1) == 0);
}


int readMinSwitchI2C(void)		// read switch from I2C for min
{
	int result;
	I2Cstart();
	I2Cwrite(addrExpanMin << 1);  // read = 1 && write = 0; so shifting the address of sensor chip to left by 1 and writing it
	I2Cwrite(0);			    // write false
	I2Cstart();				    // start the I2C
	I2Cwrite((addrExpanMin << 1) | 1);	// ??
	result = I2Cread(0);		// 0 = Nak, 1 = ACK; reading from I2C and store it into result
	I2Cstop();					// stop I2C
	return result;				// return the result value that was read from I2C
}

int readHourSwitchI2C(void)		// read switch from I2C for min
{
	int result;
	I2Cstart();
	I2Cwrite(addrExpanHour << 1);  // read = 1 && write = 0; so shifting the address of sensor chip to left by 1 and writing it
	I2Cwrite(0);			    // write false
	I2Cstart();				    // start the I2C
	I2Cwrite((addrExpanHour << 1) | 1);	// ??
	result = I2Cread(0);		// 0 = Nak, 1 = ACK; reading from I2C and store it into result
	I2Cstop();					// stop I2C
	return result;				// return the result value that was read from I2C
}




