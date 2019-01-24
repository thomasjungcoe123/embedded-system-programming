#define CLKSRCSEL (*(volatile unsigned int *)0x400fc10c)
#define PLL0CON   (*(volatile unsigned int *)0x400fc080)
#define PLL0CFG   (*(volatile unsigned int *)0x400FC084)
#define PLL0STAT (*(volatile unsigned int *)0x400FC088)
#define PLL0FEED (*(volatile unsigned int *)0x400FC08C)
#define CCLKCFG (*(volatile unsigned int *)0x400FC104)
#define PCLKSEL0 (*(volatile unsigned int *)0x400FC1A8)
#define PCLKSEL1 (*(volatile unsigned int *)0x400FC1AC)
#define CLKOUTCFG (*(volatile unsigned int *)0x400FC1C8)
#define PINSEL3 (*(volatile unsigned int *)0x4002C00C)


//#define PLLE0 0
//#define PLLC0 1

int main(void)
{

	CLKOUTCFG = (1<<8);

	PINSEL3 &= ~(1<<23);
	PINSEL3 |= (1<<22);



	// 1. Disconnect PLL0 with one feed sequence if PLL0 is already connected.
	//PLL0CON &= ~(1<<PLLC0);
	PLL0CON &= ~(1<<1);
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// 2. Disable PLL0 with one feed sequence.
	//PLL0CON &= ~(1<<PLLE0);
	PLL0CON &= ~(1<<0);
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// 3. Change the CPU Clock Divider setting to speed up operation without PLL0, if desired.
	// The CCLKCFG register controls the division of the PLL0 output before it is used by the
	// CPU.
	CCLKCFG = 0;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// 4. Write to the Clock Source Selection Control register to change the clock source if
	// needed. 00 Selects the Internal RC oscillator as the PLL0 clock source (default).
	CLKSRCSEL &= ~(1<<0) & ~(1<<1);


	// 5. Write to the PLL0CFG and make it effective with one feed sequence. The PLL0CFG
	// can only be updated when PLL0 is disabled.
	PLL0CFG = 0x10065;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// 6. Enable PLL0 with one feed sequence.
	PLL0CON |= (1<<0);
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// 8. Wait for PLL0 to achieve lock by monitoring the PLOCK0 bit in the PLL0STAT register,
	// or using the PLOCK0 interrupt, or wait for a fixed time when the input clock to PLL0 is
	// slow (i.e. 32 kHz). The value of PLOCK0 may not be stable when the PLL reference
	// frequency (FREF, the frequency of REFCLK, which is equal to the PLL input
	// frequency divided by the pre-divider value) is less than 100 kHz or greater than
	// 20 MHz. In these cases, the PLL may be assumed to be stable after a start-up time
	// has passed. This time is 500 μs when FREF is greater than 400 kHz and 200 / FREF
	// seconds when FREF is less than 400 kHz.
	while(!(PLL0STAT & (1<<26)))
	{
		// we want to wait as long as bit 26 is zero

	}

	//PLL0STAT |= (1<<26);


	// 7. Change the CPU Clock Divider setting for the operation with PLL0. It is critical to do
	// this before connecting PLL0.
	CCLKCFG = 40;

	// 9. Connect PLL0 with one feed sequence.
	PLL0CON |= (1<<1);
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	CLKOUTCFG = (1<<8);

	PINSEL3 &= ~(1<<23);
	PINSEL3 |= (1<<22);

	while(1) {}
	return 0;

}
