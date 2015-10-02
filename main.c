#include <msp430.h> 

void write_cmd(char cmd) {
	P2OUT = 0;

	// upper nibble
	P1OUT = cmd;
	__delay_cycles(10);
	P2OUT = BIT0;
	__delay_cycles(10);
	P2OUT = 0;

	// lower nibble
	__delay_cycles(10);
	P1OUT = cmd << 4;
	__delay_cycles(10);
	P2OUT = BIT0;
	__delay_cycles(10);
	P2OUT = 0;

    __delay_cycles(2000000);
    P1OUT = 0;
}

void write_data(char data) {
	P2OUT = BIT2;
	P1OUT = 0;


	// upper nibble
	P1OUT = data;
	__delay_cycles(10);
	P2OUT |= BIT0;
	__delay_cycles(10);
	P2OUT &= BIT2;

	// lower nibble
	__delay_cycles(10);
	P1OUT = data << 4;
	__delay_cycles(10);
	P2OUT |= BIT0;
	__delay_cycles(10);
	P2OUT &= BIT2;
    __delay_cycles(2000000);
    P1OUT = 0;
}

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
    DCOCTL = CALDCO_16MHZ;

    /* initialize */
    __delay_cycles(16000000); // delay 1/4 second after power up

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

    write_data(0x48); // 'H'
    write_data(0x45); // 'E'
    write_data(0x4C); // 'L'
    write_data(0x4C); // 'L'
    write_data(0x4F); // 'O'

    write_data(0x20);

    write_data(0x57);  // 'W'
    write_data(0x4F);  // 'O'
    write_data(0x52);  // 'R'
    write_data(0x4C);  // 'L'
    write_data(0x44);  // 'D'

    write_data(0x21);  // '!'

    write_cmd(BIT1); // return home, set DDRAM address to 0x00

    write_msg("hello world!");

    write_cmd(BIT5 | BIT3 | BIT2);

    P1IE = 0;

    write_cmd(BIT7 | 0x40);
    write_msg("press now");

    P1DIR &= ~BIT3;
    P1REN |= BIT3;
    P1IFG = 0;
    P1IE = BIT3;
    __enable_interrupt();
	return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void button(void) {
	write_cmd(BIT7 | 0x40);
	write_msg("just pressed");
	P1IFG = 0;
}
