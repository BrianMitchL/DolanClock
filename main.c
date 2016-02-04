//*****************************************************************************
//
// MSP432 main.c - Dolan Clock - Myles Gavic, Thomas Harren, & Brian Mitchell
//
//****************************************************************************

#include "msp.h"

struct color{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};
struct color myColors[9];
unsigned int colorState;
static int cycles;

void addColor(unsigned int index, unsigned char red, unsigned char green, unsigned char blue) {
	myColors[index].red = red;
	myColors[index].green = green;
	myColors[index].blue = blue;
}

void selectPortFunction(int port, int line, int sel10, int sel1){
	//(p,l,0,0) will set port to Digital I/O
	if(port==1){
		if(P1SEL0 & BIT(line)!=sel10){
			if(P1SEL1 & BIT(line)!=sel1){
				P1SELC|=BIT(line);
			}else{
				P1SEL0^=BIT(line);
			}
		}else{
			if(P1SEL1 & BIT(line)!=sel1)
				P1SEL1^=BIT(line);
		}
	}else{
		if(P2SEL0 & BIT(line)!=sel10){
			if(P2SEL1 & BIT(line)!=sel1){
				P2SELC|=BIT(line);
			}else{
				P2SEL0^=BIT(line);
			}
		}else{
			if(P2SEL1 & BIT(line)!=sel1)
				P2SEL1^=BIT(line);
		}
	}
}

void initColors(void){
	addColor(0,0x80,0x00,0x4F);
	addColor(1,0x2F,0x0F,0x00);
	addColor(2,0x16,0x80,0x37);
	addColor(3,0x26,0x4E,0x42);
	addColor(4,0x2F,0x0D,0x11);
	addColor(5,0x10,0x30,0xF7);
	addColor(6,0x00,0x2F,0x10);
	addColor(7,0x10,0x55,0x77);
	addColor(8,0x9F,0x06,0x50);
}

void initButtons(void){
	P1DIR&=~(BIT1&BIT4); //0 aka "in" for button on lines 1,4

	P1REN|=BIT1|BIT4;  //enaling internal pull-up/pull-down resistors
	P1OUT|=BIT1|BIT4;  //default circuit to pull-ups

	selectDIO_P1(BIT1);
	selectDIO_P1(BIT4);
}

void initLEDs(void){
	P1DIR|=BIT0;
	P2DIR|=BIT0|BIT1|BIT2;
	selectPortFunction(1,0,0,0);
	selectPortFunction(2,0,0,0);
	selectPortFunction(2,1,0,0);
	selectPortFunction(2,2,0,0);
}

void setClockFrequency(void){
	CSKEY=0x695A;       //unlock
	CSCTL1=0x00000223;  //run at 128, enable stuff for clock
	CSCLKEN=0x0000800F;
	CSKEY=0xA596;       //lock
}

void configureTimer(void){
	TA0CTL=0x0100;   //Picks clock (above), count up mode, sets internal divider, shuts timer off.

	TA0CCTL0=0x2000; //Pick compare (not capture) register, interrupt off
	TA0CCR0=0x80;  //(128)//sets the max time compare register (1,2,3 depend on this peak)
					 //interrups every milisecond

	TA0CCTL1=0x2010; //Pick compare (not capture) register, interrupt on
	TA0CCR1=0x0040;   //sets the max time compare  for this capture, will wait until overflow (will be overwritten)
	TA0CCTL2=0x2010;
	TA0CCR2=0x0040;
	TA0CCTL3=0x2010;
	TA0CCR3=0x0040;

	TA0CTL=0x0116;   //Sets counter to 0, turns counter on, enables timeout (aka overflow) interrups
}

void setColor(unsigned char red, unsigned char green, unsigned char blue)
{
	//subtract value from 0x0080 so the color can be additive
	TA0CCR1=red;
	TA0CCR2=green;
	TA0CCR3=blue;
}


void PortOneInterrupt(void) {
	unsigned short iflag=P1IV; //IV=interrupt vector
	if(iflag==0x04)//if line 1 was hit (datasheet 10.4.1)
		autoState^=1;
	if(iflag==0x0A && !autoState){//if line 4 was hit (datasheet 10.4.1)
		setColor();
		newColor();
	}

}

void TimerA0Interrupt(void) {
	unsigned short intv=TA0IV;
	if(intv==0x0E){// OE is overflow interrupt
		P2OUT&=~(BIT0|BIT1|BIT2);
		setColor(myColors[colorState].red, myColors[colorState].green, myColors[colorState].blue);

		if(++(cycles)==1000){
			cycles=0;
			if(++(colorState)==9)
				colorState=0;
		}

	}else if(intv==0x02){//red
		P2OUT|=BIT0;
	}else if(intv==0x04){//green
		P2OUT|=BIT1;
	}else if(intv==0x06){//blue
		P2OUT|=BIT2;
	}

}


void main(void){

	WDTCTL = WDTPW | WDTHOLD; //Stop watchdog timer
	initColors();
	initLEDs();
	configureTimer();
	setClockFrequency();
	NVIC_EnableIRQ(TA0_N_IRQn); //Enable TA0 interrupts using the NVIC
								//NVIC=nested vector interrupt controller

	colorState=0;
	cycles=0;

	while(1){}
}
