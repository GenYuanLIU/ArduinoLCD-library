/*
 * ArduinoLCD.c
 *
 * Created: 8/16/2018 12:00:15 PM
 * Author : Gen_Yuan_LIU
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>


void configurePins(void);							//this function configures the pins of ATMEGA328P that connects to the LCD
void writeIns(char Ins);							//this function writes Instructions to the LCD
void checkBF(void);									//this function checks the BusyFlag of the LCD, if the Flag is set, it stays in the loop until the flag is turned off.
char aquireAC(void);								//this function acquires the current address value in the Address Counter.
char writeAC(char ACaddress);						//this function writes the address value of the DDRAM or CGRAM to the Address Counter, with a return value of the updated AC value.
void setEntryMode(char ACoperMode[],char displayShift[]);	//this function sets the entry mode of the LCD, default is AC increment & without display shift.
void setCursorMode(char cursorON[],char cursorBlink[]);	//this function sets the operation mode of the cursor,default is Cursor ON & Cursor Blink.
void clearDisplay(void);							//this function clears the entire Display & write 0 to all the DDRAM & set the DDRAM address to 0.
void returnHome(void);								//this function sets the AC address to 0 without changing the contents of the DDRAM
void lcdInitiate(void);								//this function initialize the LCD 00111000
char* lcdPrint(char startingAddress,char* stringData,char* homeBeforePrint);	//this function prints string data to the LCD from the starting address,the starting address can be set to 0 by writing "yes" to the homeBeforePrint parameter.
void updateEdge(char displacement);					//this function updates the edge DDRAM address of the LCD ,after the display has been shifted.
void browseLcd(char direction[],char displacement);	// this function is used to browse the LCD by shifting the display with the given direction and displacement
char screenEdge[]={0X00,0X0F,0X40,0X4F};			//declare a global array to store the address of the screen edge & set the initial screen edge value


/* Replace with your library code */
void writeIns(char Ins)
{
	PORTB=0X00;			//set RS,R/W
	PORTD=Ins;			//send the instruction to PORTD
	PORTB=0X01;			//set EN HIGH
	_delay_ms(20);
	PORTB=0X00;			//reset the RS,R/W,EN to initial value
	PORTD=0X00;			//reset the DATA BUS
	_delay_ms(100);
	//checkBF();
}

char aquireAC(void)
{
	char AC;
	//checkBF();
	//configure PORTD as INPUT Pins
	PORTD=0X00;
	PORTD=0XFF;	
	DDRD=0X00;
	//RS->PORTB2 R/W->PORTB1 EN->PORTB0
	PORTB=0X02;			//set RS,R/W
	PORTB=0X03;			//set EN
	_delay_ms(20);
	AC=PIND &(0X7F);	//get AC value and store it in to the variable AC.
	PORTB=0X00;			//reset PORTB to initial value
	PORTD=0X00;			//turn off the internal pullup resistor of PORTD
	DDRD=0XFF;			//set PORTD to OUTPUT LOW
	return AC;
}

void checkBF(void)
{
	//configure PORTD as INPUT Pins
	PORTD=0X00;
	PORTD=0XFF;
	DDRD=0X00;	
	//check the BF every 10 microseconds, do until the BF is turned off
	while (PIND7)
	{
		PORTB=0X02;			//set RS,RW
		PORTB=0X03;			//set EN
		_delay_ms(10);
		PORTB=0X00;			//reset RS,RW,EN
		_delay_us(10);
		PORTB=0X20;
	}
	PORTB=0x00;
	PORTD=0X00;				//turn off the internal pullup resistor of PORTD
	DDRD=0XFF;				//set PORTD to OUTPUT LOW	
}

void configurePins(void)
{
	DDRD=0XFF;	//all PORTD pins are configured as OUTPUT
	PORTD=0X00;	//all PORTD pins OUTPUT LOW
	DDRB=0X07;	/*configure PORTB INPUT & OUTPUT ;
					PORTB0,1,2 are connect to EN,R/W,RS respectively, these pins should be configured as OUTPUT*/
	PORTB=0X00;	//By default, only EN,R/W,RS are connect to PORTB,these pins are all output pins and output LOW
	DDRC=0X00;	/*configure PORTC INPUT & OUTPUT
					by default, all the navigation button are connected to PORTC[0:3], these pins should be configured as INPUT with internal pullup enabled*/
	PORTC=0X0F;	//enable the internal pullup resistor for the navigation buttons	
}

char writeAC(char ACaddress)
{
	//checkBF();
	ACaddress=ACaddress|(0X80);	//combine the ACaddress to form the instructions to be sent to LCD
	writeIns(ACaddress);	//write instructions
	//checkBF(); 
}

void setEntryMode(char ACoperMode[],char displayShift[])
{
	//checkBF();
	char ins=0X06;	//by default, the entry mode is set to "Increment" & "without display shift"
	if (ACoperMode=="decrement")
	{
		ins=0X04;	//modify the instruction to "decrement"
	}
	if (displayShift=="displayshift")
	{
		ins=0X05;	//modify the instruction to "with display shift"
	}
	writeIns(ins);	//sent instructions to LCD
	//checkBF();
}

void setCursorMode(char cursorON[],char cursorBlink[])
{
	//checkBF();
	char ins=0X0F;			//by default, the cursor is turned on and is set to blink
	if (cursorON=="cursoroff")
	{
		ins=ins & (0X0D);	//modify the instruction to "CursorOFF"
	}
	if (cursorBlink=="noblink")
	{
		ins=ins & (0X0C);	//modify the instruction to "NOblink"
	}
	writeIns(ins);	//sent instruction to LCD
	//checkBF();
}

void clearDisplay(void)
{
	//checkBF();
	writeIns(0X02);
	//checkBF();
}

void returnHome(void)
{
	//checkBF();
	writeIns(0X01);
	//checkBF();
}

void lcdInitiate(void)
{
	configurePins();
	writeIns(0X0F);			//turn display on
	writeIns(0X38);			//set LCD to 8 lines interface ,2 lines, 8*5 character
	writeIns(0X01);			//clear display
}

char* lcdPrint(char startingAddress,char* stringData,char homeBeforePrint[])
{
	unsigned char ACvalue_LcdDisplacement[2];
	unsigned char LcdDisplacement=0;
	unsigned char stringIndex=0;
	if (homeBeforePrint=="yes")
	{
		returnHome();
	}
	else
	{
		startingAddress=startingAddress|(0X80);	//combine the AC address value to form the instructions
		writeIns(startingAddress);				//set DDRAM address
	}
	while (*(stringData+stringIndex)!=0)
	{
		// PORTB2->RS ; PORTB1->RW ; PORTB0->EN 
		PORTB=0X04;							//set RS,R/W
		PORTD=*(stringData+stringIndex);	//send character data to LCD
		PORTB=0X05;							//set EN
		PORTB=0X04;							//reset RS,RW,EN
		PORTD=0X00;							//reset Data Bus
		/*if (aquireAC()>=*(screenEdge+1))		//warning: this line should be review after the first test 
		{
			writeIns(0X18);						//shift the entire Display left by 1
			LcdDisplacement=LcdDisplacement+1;	//increment LcdDislplacement by 1
		}*/
		stringIndex=stringIndex+1;				//increase stringIndex by 1 
		_delay_ms(200);
	}
	/*blink the onboard LED when string is print completely*/
	DDRB=0X27;
	PORTB=0X20;
	_delay_ms(5000);
	PORTB=0X00;
	//updateEdge(LcdDisplacement);				//update the LCD edge address after the screen has been shifted
	ACvalue_LcdDisplacement[0]=aquireAC();
	ACvalue_LcdDisplacement[1]=LcdDisplacement;
	return ACvalue_LcdDisplacement;
}

void updateEdge(char displacement)
{
	//checkBF();
	displacement=displacement % 40;		//get the remaining term of displacement divided by 40
	if (displacement<0)
	{
		displacement=40+displacement;
	}
	screenEdge[0]=displacement;
	screenEdge[2]=displacement+0X40;
	screenEdge[1]=displacement+0X0F;
	if (screenEdge[1]>0X27)			//determine if the right upper edge address has exceed the DDRAM address
	{
		screenEdge[1]=screenEdge[1]-0X27;	//if exceed, minus 0X27
	}
	screenEdge[3]=displacement+0X4F;
	if (screenEdge[3]>0X67)			//determine if the right lower edge address has exceed the DDRAM address
	{
		screenEdge[3]=screenEdge[3]-0X27;	//if exceed, minus 0X27
	}
}

void browseLcd(char direction[],char displacement)
{
	//checkBF();
	if (direction=="vertical")	//direction=horizontal ->shift left or right ; direction=vertical ->shift up or down 
	{
		if (displacement>0)		//displacement>0 ->shift up ; displacement<0 ->shift down
		{
			writeAC(aquireAC()-0X40);	//shift cursor up 
		}
		else
		{
			writeAC(aquireAC()+0X40);	//shift cursor down
		}
	}
	else
	{
		if (((displacement<0X28)||(displacement>0XD7)))
		{
			if ((displacement&0X80)>>7)				//displacement>0 ->shifting left ; displacement<0 ->shifting right
			{
				displacement=40+displacement;		//shift right in the way of shifting left (ex:shifting right by 2 = shifting left by 38, since there are 40 positions in one row)
			}
			//updateEdge(displacement);				//update the screen edge address
			for (int i=0;i<displacement;i++)
			{
				writeIns(0X18);						//shift entire screen left by 1
			}
			writeAC((aquireAC()+displacement));		//when shifting the entire LCD, the cursor will follow the shifting ; this line shifts the cursor back to its original position			
		}
		else
		{
			lcdPrint(0X40,"Error:>40or<-40!","no");	//print error message if input displacement less than -40 or greater than 40
		}
	}
}