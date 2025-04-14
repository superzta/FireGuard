/*************************************************************
*       at328-0.c - Demonstrate simple I/O functions of ATmega328
*
*       Program loops turning PC0 on and off as fast as possible.
*
* The program should generate code in the loop consisting of
*   LOOP:   SBI  PORTC,0        (2 cycles)
*           CBI  PORTC,0        (2 cycles)
*           RJMP LOOP           (2 cycles)
*
* PC0 will be low for 4 / XTAL freq
* PC0 will be high for 2 / XTAL freq
* A 9.8304MHz clock gives a loop period of about 600 nanoseconds.
*
* Revision History
* Date     Author      Description
* 09/14/12 A. Weber    Initial Release
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

int main(){
    lcd_init();
    // clear the screen
    lcd_writecommand(1);
    // move to the next line
    lcd_moveto(0,0);
    // write a string
    lcd_stringout("Hello World!");
    // move to the next line
    lcd_moveto(1,0);
    // write a string
    lcd_stringout("EE109 Final Project");
    while(1){
        // move to the next line
        lcd_moveto(0,0);
        // write a string
        lcd_stringout("Hello World!");
        // move to the next line
        lcd_moveto(1,0);
        // write a string
        lcd_stringout("EE109 Final Project");

    }






    // lcd_init();	// Initialize the LCD
	

    // // Write a spash screen to the LCD
	// lcd_writecommand(1); // Clear the display
    // lcd_writecommand(0x0C);     // Display and cursor off
    // lcd_moveto(0,3);    // Move to the top left corner
    //     // Print the splash screen
    // lcd_stringout("EE109 Lab7"); 
    // lcd_moveto(1,3);    // Move to the bottom row
    // lcd_stringout("Owen Zeng");
    // //lcd_writecommand(2); // Move cursor to home
    // _delay_ms(1000);    // Delay 1 second
    // lcd_writecommand(1); // Clear the display
    return 1;

}