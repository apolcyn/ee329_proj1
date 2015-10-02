#include <msp430.h> 

int action = 0;  // global to keep track of what action the button press should take

void write_cmd(char cmd) {
	P2OUT = 0;

	// upper nibble
	P1OUT = cmd;
	__delay_cycles(10);
	P2OUT = BIT0;      // raise enable
	__delay_cycles(10);
	P2OUT = 0;   // lower enable

	// lower nibble
	__delay_cycles(10);
	P1OUT = cmd << 4;
	__delay_cycles(10);
	P2OUT = BIT0;
	__delay_cycles(10);
	P2OUT = 0;

    __delay_cycles(2000000);  // wait a long time, allow operation to complete
    P1OUT = 0; // clear P1OUT,just keeping don't care lines low
}

void write_data(char data) {
	P2OUT = BIT2;
	P1OUT = 0;


	// upper nibble
	P1OUT = data;
	__delay_cycles(10);
	P2OUT |= BIT0;     // set enable high
	__delay_cycles(10);
	P2OUT &= BIT2;     // lower enable, but keep RS high

	// lower nibble
	__delay_cycles(10);
	P1OUT = data << 4;
	__delay_cycles(10);
	P2OUT |= BIT0;
	__delay_cycles(10);
	P2OUT &= BIT2;
    __delay_cycles(2000000); // Wait a long time, allow operation to complete
    P1OUT = 0;  // clear P1OUT,just keeping don't care lines low
}

/* Writes a string of characters to DDRAM */
write_msg(char* arr) {
	while(*arr) {
		write_data(*arr++);
	}
}

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer


    P1DIR = 0xFF;
    P1OUT = 0;

    P2DIR = BIT0 | BIT1 | BIT2;
    P2OUT = 0;

    if (CALBC1_16MHZ==0xFF) // If calibration constant erased
    {
        while(1); // do not load, trap CPU!!
    }
    DCOCTL = 0; // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_16MHZ; // Set range
    DCOCTL = CALDCO_16MHZ;  // MCLK set to 16MHz

    /* initialize */
    __delay_cycles(16000000); // delay 1/4 second after power up

    /* Write the first command to put it in nibble mode */
    P2OUT = 0;
    P1OUT = BIT5;
    __delay_cycles(10);
    P2OUT = BIT0;
    __delay_cycles(10);
    P2OUT = 0;

    write_cmd(BIT2 | BIT3 | BIT5); // 2 line mode, display on

    __delay_cycles(2000000);

    write_cmd(BIT0 | BIT1 | BIT2| BIT3); // display on, cursor on, blink on
    __delay_cycles(2000000);

    write_cmd(BIT0); // display clear
    __delay_cycles(2000000);

    write_cmd(BIT1 | BIT2); // increment mode, shift off

    write_cmd(BIT1); // return home, set DDRAM address to 0x00

    write_cmd(BIT2 | BIT3 | BIT5);  // function set, put in two line mode, nibble mode

    write_msg("hello world!");

    P1IE = 0;

    write_cmd(BIT7 | 0x40);
    write_msg("press now");

    P1DIR &= ~BIT3;
    P1IE = BIT3;
    P1REN |= BIT3;
    P1OUT |= BIT3;
    P1IES |= BIT3;
    P1IFG &= ~BIT3;
    __enable_interrupt();
	return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void button(void) {
	int i = 0;

	switch(action) {
	case 0:
		write_cmd(BIT1);  // return home
		write_msg("just pressed");
		write_cmd(BIT1); // return home
		action = 1;
		break;
	case 1:
		write_cmd(BIT1);  // return home
		write_cmd(BIT7 | 0x40);  // shift cursor to lower left corner
		write_msg("pressed again");
		write_cmd(BIT1); // return home
		action = 2;
		break;
	case 2:
		write_cmd(BIT1);
		/* Cycle display, shifting to the right */
		for(i = 0; i < 40; i++) {
			 write_cmd(BIT4 | BIT3 | BIT2);
		}
		/* Cycle display, shifting to the left */
		for(i = 0; i < 40; i++) {
			 write_cmd(BIT4 | BIT3);
		}
		write_cmd(BIT1); // return home
		action = 0;
		break;
	}
	write_cmd(BIT1); // return home
	P1OUT |= BIT3; // make sure that we're still driving P1.3 high
	P1IFG = 0;     // clear any P1 pin interrupt flags that might have been set
}
