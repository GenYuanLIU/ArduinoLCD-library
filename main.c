/*
 * ArduinoLCD.c
 *
 * Created: 8/16/2018 6:10:21 PM
 * Author : Gen_Yuan_LIU
 */ 

#include <avr/io.h>
#include "LCDlibrary.c"
#include <util/delay.h>
#include <stdio.h>
#define F_CPU 16000000UL

int main(void)
{
    /* Replace with your application code */
	
	lcdInitiate();
	lcdPrint(0X03,"Hello Arduino...","yes");
	lcdPrint(0X40,"Hello HD44780...","no");
}

