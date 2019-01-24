/*
===============================================================================
 Name        : HW_2.c
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
//#define FIO0DIR (*(struct GPFOstruct *)0x2009C000)
//#define FIO0MASK (*(struct GPFOstruct  *)0x2009C010)
//#define FIO0PIN (*(struct GPFOstruct  *)0x2009C014)
//#define FIO0SET (*(struct GPFOstruct  *)0x2009C018)
//#define FIO0CLR (*(struct GPFOstruct  *)0x2009C01C)


// TODO: insert other definitions and declarations here
struct GPFOstruct {

	volatile unsigned int FIODIR;
	int junk[3];
	volatile unsigned int FIOMASK;
	volatile unsigned int FIOPIN;
	volatile unsigned int FIOSET;
	volatile unsigned int FIOCLR;

};

#define FIO ((struct GPFOstruct *)0x2009c000)
//#define FIO0DIR FIO[0].FIODIR

void wait_ticks(int count); // helper function for wait
void reset_game();			// resetting the game
void p1_turn();				// player 1's turn to choose the space for tick tac toe game
void p2_turn();				// player 2's turn
void p1_win();				// p1's win checking function
void p2_win();				// p2's win checking function
//void haveWon();

// switch input state checker
_Bool p1swTopRight();		// detecting p1 switch being pressed on top right
_Bool p1swMidRight();		// etc
_Bool p1swButtomRight();
_Bool p1swTopMid();
_Bool p1swMidMid();
_Bool p1swButtomMid();
_Bool p1swTopLeft();
_Bool p1swMiddleLeft();
_Bool p1swButtomLeft();

_Bool p2swTopRight();		// detecting p2 switch being pressed on top right
_Bool p2swMidRight();		// etc
_Bool p2swButtomRight();
_Bool p2swTopMid();
_Bool p2swMidMid();
_Bool p2swButtomMid();
_Bool p2swTopLeft();
_Bool p2swMiddleLeft();
_Bool p2swButtomLeft();

// led output state checker
_Bool isP1LEDTopRight();	// outputing LED based on player one's switching being pressed (TopRight) and also keeps the state of the LED's
_Bool isP1LEDMidRight();	// etc
_Bool isP1LEDButtomRight();
_Bool isP1LEDTopMid();
_Bool isP1LEDMidMid();
_Bool isP1LEDButtomMid();
_Bool isP1LEDTopLeft();
_Bool isP1LEDMiddleLeft();
_Bool isP1LEDButtomLeft();

_Bool isP2LEDTopRight();	// outputing LED based on player two's switching being pressed (TopRight) and also keeps the state of the LED's
_Bool isP2LEDMidRight();	// etc
_Bool isP2LEDButtomRight();
_Bool isP2LEDTopMid();
_Bool isP2LEDMidMid();
_Bool isP2LEDButtomMid();
_Bool isP2LEDTopLeft();
_Bool isP2LEDMiddleLeft();
_Bool isP2LEDButtomLeft();

_Bool won = 0;				// checking to see if game is won, which is defaulted to false

_Bool p1LEDTopRight = 0;	// current state of the LED
_Bool p1LEDMidRight = 0;	// etc
_Bool p1LEDButtomRight = 0;
_Bool p1LEDTopMid = 0;
_Bool p1LEDMidMid = 0;
_Bool p1LEDButtomMid = 0;
_Bool p1LEDTopLeft = 0;
_Bool p1LEDMiddleLeft = 0;
_Bool p1LEDButtomLeft = 0;

_Bool p2LEDTopRight = 0;
_Bool p2LEDMidRight = 0;
_Bool p2LEDButtomRight = 0;
_Bool p2LEDTopMid = 0;
_Bool p2LEDMidMid = 0;
_Bool p2LEDButtomMid = 0;
_Bool p2LEDTopLeft = 0;
_Bool p2LEDMiddleLeft = 0;
_Bool p2LEDButtomLeft = 0;

// main
int main(void) {

	// TODO: insert code here
	// input switch set to 0 on bits 8, 7, 6, 5, 4, 3, 2, 1, 0 on port 2
	// rest of the bit location are set to 1 (NOT pin)
	FIO[2].FIODIR &= ~(1<<8) & ~(1<<7) & ~(1<<6) & ~(1<<5) & ~(1<<4) & ~(1<<3) & ~(1<<2) & ~(1<<1) & ~(1<<0);

	// output LED set to 1 on pins 9, 8, 7, 6, 0, 1, 18, 17, 15, 16, 23, 24, 25, 26, 2, 3, 21, 22 on port 0
	// rest of bit locations are set to 0
	FIO[0].FIODIR |= (1<<9) | (1<<8) | (1<<7) | (1<<6) | (1<<0) | (1<<1) | (1<<18) | (1<<17) | (1<<15) |
			         (1<<16) | (1<<23) | (1<<24) | (1<<25) | (1<<26) | (1<<2) | (1<<3) | (1<<21) | (1<<22);

	// resetting LED before game start
	reset_game();

	// winning state defaulted to false so it would run
	won = 0;

	// continue until the winning condition is met or the game has no more options to be inputed
	while(1)
	{
		// turn based tick-tac-toe
		// turn 1
		p1_turn();
		p2_turn();
		// turn 2
		p1_turn();
		p2_turn();
		// turn 3
		p1_turn();
		// p1 won game?
		p1_win();
		if (won == 1)
		{
			wait_ticks(500000);
			reset_game();
			break;
		}

		p2_turn();
		// p2 won game?
		p2_win();
		if (won == 1)
		{
			wait_ticks(500000);
			reset_game();
			break;
		}

		p1_turn();
		// p1 won game?
		p1_win();
		if (won == 1)
		{
			wait_ticks(500000);
			reset_game();
			break;
		}

		p2_turn();
		// p2 won game?
		p2_win();
		if (won == 1)
		{
			wait_ticks(500000);
			reset_game();
			break;
		}

		p1_turn();
		// p1 won game?
		p1_win();
		if (won == 1)
		{
			wait_ticks(500000);
			reset_game();
			break;
		}

		// pause before game ends and restart
		wait_ticks(500000);
		reset_game();
	}

	// end of main
	return 0;
}

// helper function for wait function - runs as long as required by the parameter
void wait_ticks(int count)
{
	volatile int ticks = count;
	while (ticks>0)
		ticks--;
}

// refreshing LED to OFF mode
void reset_game()
{
	// resetting LEDs -- LED OFF
	FIO[0].FIOPIN &= ~(1<<9) & ~(1<<8) & ~(1<<7) & ~(1<<6) & ~(1<<0) & ~(1<<1) & ~(1<<18) & ~(1<<17) & ~(1<<15) & ~(1<<16)
		          & ~(1<<23) & ~(1<<24) & ~(1<<25) & ~(1<<26) & ~(1<<2) & ~(1<<3) & ~(1<<21) & ~(1<<22);

	// also reset the state of LEDs for both player one and player two
	p1LEDTopRight = 0;
	p1LEDMidRight = 0;
	p1LEDButtomRight = 0;
	p1LEDTopMid = 0;
	p1LEDMidMid = 0;
	p1LEDButtomMid = 0;
	p1LEDTopLeft = 0;
	p1LEDMiddleLeft = 0;
	p1LEDButtomLeft = 0;

	p2LEDTopRight = 0;
	p2LEDMidRight = 0;
	p2LEDButtomRight = 0;
	p2LEDTopMid = 0;
	p2LEDMidMid = 0;
	p2LEDButtomMid = 0;
	p2LEDTopLeft = 0;
	p2LEDMiddleLeft = 0;
	p2LEDButtomLeft = 0;
}

// Input helper function~~~
// player one's input detectors
_Bool p1swTopRight()
{
	// looking to see if the port 2 and bit 8 is pressed and set to 0 (this would normally be 1
	// until the switch is pressed since the switch is set as active low mode
	return (((FIO[2].FIOPIN >> 8) & 1) == 0);
}

_Bool p1swMidRight()
{
	// the same as above but in bit 7
	return (((FIO[2].FIOPIN >> 7) & 1) == 0);
}

_Bool p1swButtomRight()
{
	// the same as above but in bit 6
	return (((FIO[2].FIOPIN >> 6) & 1) == 0);
}

_Bool p1swTopMid()
{
	// the same as above but in bit 5
	return (((FIO[2].FIOPIN >> 5) & 1) == 0);
}

_Bool p1swMidMid()
{
	// the same as above but in bit 4
	return (((FIO[2].FIOPIN >> 4) & 1) == 0);
}

_Bool p1swButtomMid()
{
	// the same as above but in bit 3
	return (((FIO[2].FIOPIN >> 3) & 1) == 0);
}

_Bool p1swTopLeft()
{
	// the same as above but in bit 2
	return (((FIO[2].FIOPIN >> 2) & 1) == 0);
}

_Bool p1swMiddleLeft()
{
	// the same as above but in bit 1
	return (((FIO[2].FIOPIN >> 1) & 1) == 0);
}

_Bool p1swButtomLeft()
{
	// the same as above but in bit 0
	return (((FIO[2].FIOPIN >> 0) & 1) == 0);
}


// player two's switch input detector
_Bool p2swTopRight()
{
	// looking to see if the port 2 and bit 8 is pressed and set to 0 (this would normally be 1
	// until the switch is pressed since the switch is set as active low mode
	return (((FIO[2].FIOPIN >> 8) & 1) == 0);
}
_Bool p2swMidRight()
{

	// the same as above but in bit 7
	return (((FIO[2].FIOPIN >> 7) & 1) == 0);
}

_Bool p2swButtomRight()
{
	// the same as above but in bit 6
	return (((FIO[2].FIOPIN >> 6) & 1) == 0);
}

_Bool p2swTopMid()
{
	// the same as above but in bit 5
	return (((FIO[2].FIOPIN >> 5) & 1) == 0);
}

_Bool p2swMidMid()
{
	// the same as above but in bit 4
	return (((FIO[2].FIOPIN >> 4) & 1) == 0);
}

_Bool p2swButtomMid()
{
	// the same as above but in bit 3
	return (((FIO[2].FIOPIN >> 3) & 1) == 0);
}

_Bool p2swTopLeft()
{
	// the same as above but in bit 2
	return (((FIO[2].FIOPIN >> 2) & 1) == 0);
}

_Bool p2swMiddleLeft()
{
	// the same as above but in bit 1
	return (((FIO[2].FIOPIN >> 1) & 1) == 0);
}

_Bool p2swButtomLeft()
{
	// the same as above but in bit 0
	return (((FIO[2].FIOPIN >> 0) & 1) == 0);
}

// OutPut Helper functions~~~
// player one LED outputer
_Bool isP1LEDTopRight()
{
	p1LEDTopRight = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 9
	return FIO[0].FIOPIN |= (1<<9);
}

_Bool isP1LEDMidRight()
{
	p1LEDMidRight = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 7
	return FIO[0].FIOPIN |= (1<<7);
}

_Bool isP1LEDButtomRight()
{
	p1LEDButtomRight = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 0
	return FIO[0].FIOPIN |= (1<<0);
}

_Bool isP1LEDTopMid()
{
	p1LEDTopMid = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 18
	return FIO[0].FIOPIN |= (1<<18);
}

_Bool isP1LEDMidMid()
{
	p1LEDMidMid = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 15
	return FIO[0].FIOPIN |= (1<<15);
}

_Bool isP1LEDButtomMid()
{
	p1LEDButtomMid = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 23
	return FIO[0].FIOPIN |= (1<<23);
}

_Bool isP1LEDTopLeft()
{
	p1LEDTopLeft = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 25
	return FIO[0].FIOPIN |= (1<<25);
}

_Bool isP1LEDMiddleLeft()
{
	p1LEDMiddleLeft = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 2
	return FIO[0].FIOPIN |= (1<<2);
}

_Bool isP1LEDButtomLeft()
{
	p1LEDButtomLeft = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 21
	return FIO[0].FIOPIN |= (1<<21);
}


// player 2 LED outputer
_Bool isP2LEDTopRight()
{
	p2LEDTopRight = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 8
	return FIO[0].FIOPIN |= (1<<8);
}

_Bool isP2LEDMidRight()
{
	p2LEDMidRight = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 6
	return FIO[0].FIOPIN |= (1<<6);
}

_Bool isP2LEDButtomRight()
{
	p2LEDButtomRight = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 1
	return FIO[0].FIOPIN |= (1<<1);
}

_Bool isP2LEDTopMid()
{
	p2LEDTopMid = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 17
	return FIO[0].FIOPIN |= (1<<17);
}

_Bool isP2LEDMidMid()
{
	p2LEDMidMid = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 16
	return FIO[0].FIOPIN |= (1<<16);
}

_Bool isP2LEDButtomMid()
{
	p2LEDButtomMid = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 24
	return FIO[0].FIOPIN |= (1<<24);
}

_Bool isP2LEDTopLeft()
{
	p2LEDTopLeft = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 26
	return FIO[0].FIOPIN |= (1<<26);
}

_Bool isP2LEDMiddleLeft()
{
	p2LEDMiddleLeft = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 3
	return FIO[0].FIOPIN |= (1<<3);
}

_Bool isP2LEDButtomLeft()
{
	p2LEDButtomLeft = 1;
	// turns on LED that is connected to the pin that has port 0 and pin 22
	return FIO[0].FIOPIN |= (1<<22);
}


// player one's turn for tick-tac-toe
void p1_turn()
{
	// infinite loop until return condition is initiated
	while(1)
	{
		// checking for switch being pressed by the user
		if ( p1swTopRight() )
		{
			// if only if both LED is off, this condition will be initated
			if ( (p1LEDTopRight == 0) && (p2LEDTopRight == 0) )
			{
				// for the machine to have waited for the users when the switch is pressed
				while (p1swTopRight()) { }
				// output of the LED on the appropriate location
				isP1LEDTopRight();
				return;
			}
		}
		else if ( p1swMidRight() ) // the same as above
		{
			if ( (p1LEDMidRight == 0) && (p2LEDMidRight == 0) )
			{
				isP1LEDMidRight();
				while (p1swMidRight()) { }
				return;
			}
		}
		else if ( p1swButtomRight() )
		{
			if ( (p1LEDButtomRight == 0) && (p2LEDButtomRight == 0) )
			{
				isP1LEDButtomRight();
				while (p1swButtomRight()) { }
				return;
			}
		}
		else if ( p1swTopMid() )
		{
			if ( (p1LEDTopMid == 0) && (p2LEDTopMid == 0) )
			{
				isP1LEDTopMid();
				while (p1swTopMid()) { }
				return;
			}
		}
		else if ( p1swMidMid() )
		{
			if ( (p1LEDMidMid == 0) && (p2LEDMidMid == 0) )
			{
				isP1LEDMidMid();
				while (p1swMidMid()) { }
				return;
			}
		}
		else if ( p1swButtomMid() )
		{
			if ( (p1LEDButtomMid == 0) && (p2LEDButtomMid == 0) )
			{
				isP1LEDButtomMid();
				while (p1swButtomMid()) { }
				return;
			}
		}
		else if ( p1swTopLeft() )
		{
			if ( (p1LEDTopLeft == 0) && (p2LEDTopLeft == 0) )
			{
				isP1LEDTopLeft();
				while (p1swTopLeft()) { }
				return;
			}
		}
		else if ( p1swMiddleLeft() )
		{
			if ( (p1LEDMiddleLeft == 0) && (p2LEDMiddleLeft == 0) )
			{
				isP1LEDMiddleLeft();
				while (p1swMiddleLeft()) { }
				return;
			}
		}
		else if ( p1swButtomLeft() )
		{
			if ( (p1LEDButtomLeft == 0) && (p2LEDButtomLeft == 0) )
			{
				isP1LEDButtomLeft();
				while (p1swButtomLeft()) { }
				return;
			}
		}
	}
}

// player's two's turn on my tick-tac-toe game
void p2_turn()
{

	while(1)
	{
		// the same as above
		if ( p2swTopRight() )
		{
			if ( (p2LEDTopRight == 0) && (p1LEDTopRight ==0) )
			{
				isP2LEDTopRight();
				while (p2swTopRight()) { }
				return;
			}
		}
		else if ( p2swMidRight() )
		{
			if ( (p2LEDMidRight == 0) && (p1LEDMidRight == 0) )
			{
				isP2LEDMidRight();
				while (p2swMidRight()) { }
				return;
			}
		}
		else if ( p2swButtomRight() )
		{
			if ( (p2LEDButtomRight == 0) && (p1LEDButtomRight == 0) )
			{
				isP2LEDButtomRight();
				while (p2swButtomRight()) { }
				return;
			}
		}
		else if ( p2swTopMid() )
		{
			if ( (p2LEDTopMid == 0) && (p1LEDTopMid ==0) )
			{
				isP2LEDTopMid();
				while ( p2swTopMid() ) { }
				return;
			}
		}
		else if ( p2swMidMid() )
		{
			if ( (p2LEDMidMid == 0) && (p1LEDMidMid ==0) )
			{
				isP2LEDMidMid();
				while ( p2swMidMid() ) { }
				return;
			}
		}
		else if (p2swButtomMid())
		{
			if ( (p2LEDButtomMid == 0) && (p1LEDButtomMid ==0) )
			{
				isP2LEDButtomMid();
				while ( p2swButtomMid() ) { }
				return;
			}
		}
		else if (p2swTopLeft() )
		{
			if ( (p2LEDTopLeft == 0) && (p1LEDTopLeft ==0) )
			{
				isP2LEDTopLeft();
				while ( p2swTopLeft() ) { }
				return;
			}
		}
		else if (p2swMiddleLeft() )
		{
			if ( (p2LEDMiddleLeft == 0) && (p1LEDMiddleLeft ==0) )
			{
				isP2LEDMiddleLeft();
				while (p2swMiddleLeft()) { }
				return;
			}
		}
		else if ( p2swButtomLeft() )
		{
			if ( (p2LEDButtomLeft == 0) && (p1LEDButtomLeft ==0) )
			{
				isP2LEDButtomLeft();
				while (p2swButtomLeft()) { }
				return;
			}
		}
	}
}

void p1_win()
{
	// winning state for player one
	// total of 8 conditions that can be used to win the game
	if ( (p1LEDTopRight == 1) && (p1LEDTopMid == 1) && (p1LEDTopLeft == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDMidRight == 1) && (p1LEDMidMid == 1) && (p1LEDMiddleLeft == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDButtomRight == 1) && (p1LEDButtomMid == 1) && (p1LEDButtomLeft == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDTopLeft == 1) && (p1LEDMiddleLeft == 1) && (p1LEDButtomLeft == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDTopMid == 1) && (p1LEDMidMid == 1) && (p1LEDButtomMid == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDTopRight == 1) && (p1LEDMidRight == 1) && (p1LEDButtomRight == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDTopLeft == 1) && (p1LEDMidMid == 1) && (p1LEDButtomRight == 1) )
	{
		won = 1;
	}
	else if ( (p1LEDButtomLeft == 1) && (p1LEDMidMid==1) && (p1LEDTopRight==1) )
	{
		won = 1;
	}
	else {
		won = 0;
	}
}

void p2_win()
{
	// the same for the player two
	if ( (p2LEDTopRight == 1) && (p2LEDTopMid == 1) && (p2LEDTopLeft == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDMidRight == 1) && (p2LEDMidMid == 1) && (p2LEDButtomLeft == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDButtomRight == 1) && (p2LEDButtomMid == 1) && (p2LEDButtomLeft == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDTopLeft == 1) && (p2LEDMiddleLeft == 1) && (p2LEDButtomLeft == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDTopMid == 1) && (p2LEDMidMid == 1) && (p2LEDButtomMid == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDTopRight == 1) && (p2LEDMidRight == 1) && (p2LEDButtomRight == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDTopLeft == 1) && (p2LEDMidMid == 1) && (p2LEDButtomRight == 1) )
	{
		won = 1;
	}
	else if ( (p2LEDButtomLeft == 1) && (p2LEDMidMid==1) && (p2LEDTopRight==1) )
	{
		won = 1;
	}
	else {
		won = 0;
	}
}

//void haveWon()
//{
//	if (won == 1)
//	{
//		wait_ticks(500000);
//		//exit();
//	}
//}
