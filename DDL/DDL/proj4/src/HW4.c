/*
===============================================================================
 Name        : HW4.c
 Author      : $(author) Thomas Jung & Anouar
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#define I2C0CONSET (*(volatile unsigned int *) 0x4001C000)	// I2C Control Set Register for master and slave operation between microchip and the other chips such as Tsensor and I/O expander chips
#define I2C0CONCLR (*(volatile unsigned int *) 0x4001C018)	// I2C Control Clear Register for controlling the corresponding bit in the I2C control register, which operates when clears
#define I2C0DAT    (*(volatile unsigned int *) 0x4001C008)	// I2C Data Register for data storage when reading and writing
#define I2C0STAT   (*(volatile unsigned int *) 0x4001C004)	// I2C Status Register for providing detailed status codes that allow software to determine the next action needed.
#define PCONP    (*(volatile unsigned int *) 0x400FC0C4)	// Power Control for Peripherals register (PCONP) using internal 4MHz clock and power control
#define PCLKSEL0 (*(volatile unsigned int *) 0x400FC1A8)	// Peripheral Clock Selection register that determines clock rate.
#define PINSEL1  (*(volatile unsigned int *) 0x4002C004)	// Pin function select register for controling the functions of the upper half of Port 0
#define PINMODE0  (*(volatile unsigned int *) 0x4002C040)	// Pin Mode select register 0 -- controls pull-up/pull-down resistor configuration for Port 0 pins 0 to 15
#define I2C0SCLL  (*(volatile unsigned int *) 0x4001C014)	// SCL Duty Cycle Register Low Half Word -- Determines the low time of the I2C clock
#define I2C0SCLH  (*(volatile unsigned int *) 0x4001C010)	// SCH Duty Cycle Register High Half Word -- Determines the high time of the I2C clock.

// address of sonsor -> 1001(A2)(A1)(A0)
const int addrSensor = 0b1001000;	// slected address of the sensor chip
// read = 1 && write = 0;
const int addrExpan = 0b0100000;	// selected address of the I/O Expander chip
_Bool switchPressed = 0;			// switch state

int temperature;					// holds value of the temperature
int celChoice = 0;					// choice of celcius
int felChoice = 0;					// choice of fahrenheight

void I2Cstart(void);				// methods that start I2C
void I2Cstop(void);					// methods that start I2S
int I2Cread(int ack);				// methods for write
void I2Cwrite(int data);			// write to I2C
void initializeI2C(void);			// initializes I2C
int readTemperature(void);			// reads temp from sensor chip
void writeTemperatureLeft(int value);	// writes to I/O Expander chip the temperature that was read from sensor chip  (10's digit)
void writeTemperatureRight(int value);  // // writes to I/O Expander chip the temperature that was read from sensor chip (1's digit)
void configOutput(void);			// configures I/O expander chip to be output
int display1(int value);			// display to 7segment
int display(int value);				// the same
int celToFahr(int cel);				// converts the temperature into Fahr


/*************************************************************************/
/*************************    main   *************************************/
/*************************************************************************/
int main(void)
{
	int celChoice = 1;				// starts with celcious
	int felChoice = 0;				// fahr off at the start
//	switchPressed = 0;

	initializeI2C();		// initialize the I2C
	configOutput();			// configurate the I/O as output

	while (1)
	{
		temperature = readTemperature();		// readTemperature from LB75 and store the result

		if(readSwitchState() == 1)				// if the switch is pressed
		{
			if (celChoice == 0)					// change the current state of the temperature
			{
				celChoice = 1;
			} else {
				celChoice = 0;
			}

			if (felChoice == 0)
			{
				felChoice = 1;
			} else {
				felChoice = 0;
			}
		}
		while (readSwitchState() == 1) {}		// wait until the switch is released


		if (felChoice == 1 && celChoice == 0)   // if in Fahr, convert the temp to Fahr and write to the chip
		{
			writeTemperatureRight(celToFahr(temperature));	// write 1's digit
			writeTemperatureLeft(celToFahr(temperature));	// write 10th digit
		}
		if (celChoice == 1 && felChoice == 0)	// if in Cel, just use the temp as the parameter
		{
			writeTemperatureRight(temperature);		// write 1's digit
			writeTemperatureLeft(temperature);		// write 10th digit
		}
	}
    return 0;		// end of main
}

/******************************************************************/
/******************* helper functions *****************************/
/******************************************************************/

// function for initializing I2C
void initializeI2C(void)
{

	// duty high and low (cuts our 1MHz into 100kHz) because fout = 1Mhz/ (I2C0SCLL+I2C0SCLH) = 100kHz
	I2C0SCLL = 0x05;	// 5 low time
	I2C0SCLH = 0x05;	// 5 high time

	// enabling I2C
	I2C0CONSET = (1<<6);	// I2EN, I2C interface enable. 
	
	// basic config.
	// 1. Power: In the PCONP register (Table 46), set bit PCI2C0/1/2.
	PCONP |= (1<<7);	// enabling PCI2C0 -- The I2C0 interface power/clock control bit of PCONP

	// 2. Clock: In PCLKSEL0 select PCLK_I2C0; in PCLKSEL1 select PCLK_I2C1 or PCLK_I2C2 (see Section 4.7.3).
	PCLKSEL0 &= ~(1<<0);	// Peripheral clock selection for I2C0. (00)
	PCLKSEL0 &= ~(1<<1);   	//  00 --> cclk/4

	// 3. Pins: Select I2C0, I2C1, or I2C2 pins through the PINSEL registers. Select the pin modes for the port pins with I2C1 or I2C2 functions through the PINMODE registers
	// (no pull-up, no pull-down resistors) and the PINMODE_OD registers (open drain) (See Section 8.5).
	PINSEL1 |= (1<<22);		// GPIO Port 0.27 has been selected for SDA0 with when function 01
	PINSEL1 &= ~(1<<23);	

	PINSEL1 |= (1<<24);		// GPIO Port 0.28 has been selected for SCL0 with when function 01
	PINSEL1 &= ~(1<<25);
}

// function for configuration of the I/O expansion chip, deciding the direction of the data flow.
// basically setting both side of the Expander chip to be output.
void configOutput(void)
{
	I2Cstart();
	I2Cwrite(addrExpan << 1); // address of expander chip shifted to left by 1. (// read = 1 && write = 0;))
	// input/output enable
	I2Cwrite(0x00);			  // IODIRA set
	// fix which am i using **
	I2Cwrite(0b10000000);	  // write to I/O Expander chip
	I2Cstop();

	I2Cstart();
	I2Cwrite(addrExpan << 1);
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

// function for writing to I2C
void I2Cwrite(int data)
{
	I2C0DAT = data;	// ! bit manipulation
	I2C0CONCLR = (1<<3);	// clear SI

	while((I2C0CONSET & (1 << 3)) == 0) { }	// waiting for si bit to be enabled

//	while(((I2C0CONSET >> 3) & 1) == 0) { }

	switch(I2C0STAT & 0xF8) {
		case 0x18:
		case 0x28:
		case 0x40:
		case 0x50: return 1;
		default: return 0;

	}
}

// address of sonsor -> 1001(A2)(A1)(A0)
//int addrSensor = 0b1001000;
int readTemperature(void)
{
	int result;
	I2Cstart();	
	I2Cwrite(addrSensor << 1);  // read = 1 && write = 0; so shifting the address of sensor chip to left by 1 and writing it
	I2Cwrite(0);			    // write false
	I2Cstart();				    // start the I2C
	I2Cwrite((addrSensor << 1) | 1);	// ??
	result = I2Cread(0);		// 0 = Nak, 1 = ACK; reading from I2C and store it into result
	I2Cstop();					// stop I2C
	return result;				// return the result value that was read from I2C
}

// function for reading the switch state
int readSwitchState(void)
{
	int result;
	I2Cstart();
	// I/O Expander chip
	I2Cwrite(addrExpan << 1);
	// GPIOB
	I2Cwrite(0x12);		// GPIOA
	I2Cstart();
	// why i shifted?
	I2Cwrite((addrExpan << 1) | 1);
	result = I2Cread(0);	// 0 = Nak, 1 = ACK
	I2Cstop();
	// returns the state of the switch
	return ((result >> 7) & 1); // returns the state of the switch pressed/nah
}

// function for writing the temp of 10's digit value
// read = 1 && write = 0;
//int addrExpan = 0b0100000;
void writeTemperatureLeft(int value)
{
	I2Cstart();
	I2Cwrite( (addrExpan << 1)|0 );	// shift it by 1 and and it with 0 since we are writing
	I2Cwrite(0x13);					// GPIOB

	int temp = display(value / 10);	// display function called with 10's digit value of the temp
	I2Cwrite(temp);
	//	I2Cwrite(display(value / 10));
	I2Cstop();
}

// function for writing the temp of 1's digit value
void writeTemperatureRight(int value)
{
	I2Cstart();
	I2Cwrite( (addrExpan << 1) | 0 );   // the same as the above
	I2Cwrite(0x12);						// GPIOA

	int temp = display(value % 10);		// display function called with 1's digit value of the temp
	I2Cwrite(temp);
	// I2Cwrite(display(value % 10));
	I2Cstop();
}

// we have anode
int display1(int value)
{
	int hexValue;
	if(value==0) { hexValue = 0x01; } // 0b0000001
	if(value==1) { hexValue = 0x4F; } // 0b1001111
	if(value==2) { hexValue = 0x12; } // 0b0010010
	if(value==3) { hexValue = 0x06; } // 0b0000110
	if(value==4) { hexValue = 0x4C; } // 0b1001100
	if(value==5) { hexValue = 0X24; } // 0b0100100
	if(value==6) { hexValue = 0X20; } // 0b0100000
	if(value==7) { hexValue = 0X0F; } // 0b0001111
	if(value==8) { hexValue = 0X00; } // 0b0000000
	if(value==9) { hexValue = 0X0C; } // 0b0001100
	return hexValue;
}

// TODO: fix the bits
// port A 0~6
// port B 1~7  --> needs to be 0~6 and 7bit used as the switch.
// rearange wires and also configure the right pins


// TODO: reading switch is very similar to reading temp

int display(int value){
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

// function for converting temperature of celcius to Fahrenheight
int celToFahr(int cel){
	// calculated in float to for float division.
	float tempCel = cel;
	float convFactor = (float)9/5;
	// stored in integer after
	int tempFahr = (tempCel * convFactor)+32;
	int Fahr = tempFahr;
	return Fahr;
}









