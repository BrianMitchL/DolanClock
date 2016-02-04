//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"

struct color{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};
struct color myColors[12];
unsigned int colorState=0;
static int cycles=0;
static int second=0;
static int hour=5;
static int hourBlink;
//unsigned int setMode;
//unsigned int settingHour;


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
	addColor(0,0x80,0x80,0x80); // white
	addColor(1,0x80,0x55,0x55);
	addColor(2,0x80,0x2A,0x2A);

	addColor(3,0x80,0x00,0x00); // red
	addColor(4,0x55,0x2A,0x00);
	addColor(5,0x2A,0x55,0x00);

	addColor(6,0x00,0x80,0x00); // green
	addColor(7,0x00,0x55,0x2A);
	addColor(8,0x00,0x06,0x55);

	addColor(9,0x00,0x00,0x80); // blue
	addColor(10,0x2A,0x2A,0x80);
	addColor(11,0x55,0x55,0x80);
}

void initButtons(void){
	P1DIR&=~(BIT1&BIT4); //0 aka "in" for button on lines 1,4

	P1REN|=BIT1|BIT4;  //enaling internal pull-up/pull-down resistors
	P1OUT|=BIT1|BIT4;  //default circuit to pull-ups
	selectPortFunction(1,1,0,0);
	selectPortFunction(1,4,0,0);
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
	TA0CCR1=0x0080;   //sets the max time compare  for this capture, will wait until overflow (will be overwritten)
	TA0CCTL2=0x2010;
	TA0CCR2=0x0080;
	TA0CCTL3=0x2010;
	TA0CCR3=0x0080;

	TA0CTL=0x0116;   //Sets counter to 0, turns counter on, enables timeout (aka overflow) interrups
}

void setColor(unsigned char red, unsigned char green, unsigned char blue)
{
	//subtract value from 0x0080 so the color can be additive
	TA0CCR1=0x80-red;
	TA0CCR2=0x80-green;
	TA0CCR3=0x80-blue;
}


void PortOneInterrupt(void) {
	//	unsigned short iflag=P1IV;
	//	if(!setMode){
	//		setMode=1;
	//		P1OUT&=~BIT0
	//		P2OUT&=~(BIT0|BIT1|BIT2)
	//
	//	}
	//	if(iflag==0x04)//S1@P1.1(datasheet 10.4.1)
	//		autoState^=1;
	//	if(iflag==0x0A && !autoState){//S2@P1.4
	//		setColor();
	//		newColor();
	//	}
	//
}

void resetHourBlink(void) {
	if(hour==0){
		hourBlink=24;
	}else{
		hourBlink = hour*2;
	}
}



void TimerA0Interrupt(void) {
	unsigned short intv=TA0IV;
	//	if(setMode){
	//		if(settingHour){
	//
	//		}else{ //setting minutes
	//
	//		}
	//	}else{//tick the time
	//
	//
	//	}
	//
	//
	//		if(intv==0x0E){// OE is overflow interrupt
	//			if(++(cycles)==1000){
	//				cycles=0;
	//
	//				if(++(colorState)==9)
	//					colorState=0;
	//			}
	//
	//		}else if(intv==0x02){//red
	//			P2OUT|=BIT0;
	//		}else if(intv==0x04){//green
	//			P2OUT|=BIT1;
	//		}else if(intv==0x06){//blue
	//			P2OUT|=BIT2;
	//		}
	//
	//	}



	if(intv==0x0E){// OE is overflow interrupt
		P2OUT&=~(BIT0|BIT1|BIT2);
		setColor(myColors[colorState].red, myColors[colorState].green, myColors[colorState].blue);

		if((++cycles)%500 == 0) { //every half second
			if(cycles == 1000) { //every second
				cycles = 0;
				if((++second) % 300 == 0) { // every 5 mintues
					colorState++;
					if(second == 3600) { // every hour
						colorState = 0;
						second = 0;
						if(++hour == 12) { // every 12 hours
							hour = 0;
						}
					}
				}
			}//use every half second to display hour blinks
			if(second%15==0){
				resetHourBlink();
			}
 			if((--hourBlink)>0){
				P1OUT^=BIT0;
			}else{
				P1OUT&=~BIT0;
			}
		}

	} else if(intv==0x02 ){//red
		P2OUT|=BIT0;
	} else if(intv==0x04) {//green
		P2OUT|=BIT1;
	} else if(intv==0x06) {//blue
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
	//hour=3;


//	setMode=1;
//	settingHour=1;

	while(1){}
}
