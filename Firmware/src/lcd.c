/*
  lcd.c - Routines for sending data and commands to the LCD shield
  Modified for serial communication using a single data pin
*/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"                // Declarations of the LCD functions

/* This function not declared in lcd.h since
   should only be used by the routines in this file. */
void lcd_writebit(unsigned char);

/* Define masks for the bits in Port D */
#define DATA_PIN (1 << PD2)    // Single data pin on PD2
#define ENABLE_PIN (1 << PD4)  // Enable pin on PD4
#define RS_PIN (1 << PB0)      // Register select pin remains on PB0

/*
  lcd_init - Initialize the LCD display for serial communication
*/
void lcd_init(void)
{
    // Set the DDR register bits for the pins we're using
    DDRB |= RS_PIN;           // RS pin as output
    DDRD |= DATA_PIN | ENABLE_PIN;  // Data and Enable pins as outputs

    _delay_ms(15);            // Delay at least 15ms after power on
    
    // Initialize in 8-bit mode first
    // Send 0x30 three times
    for (int i = 0; i < 3; i++) {
        // Send each bit of 0x30 (0b00110000)
        for (int bit = 7; bit >= 0; bit--) {
            lcd_writebit((0x30 >> bit) & 0x01);
        }
        
        if (i == 0) _delay_ms(5);      // First time: wait > 4.1ms
        else if (i == 1) _delay_us(120); // Second time: wait > 100us
        // Third time: no delay needed
    }
    
    // Switch to 8-bit mode with 1-line display
    // Send 0x20 (0b00100000)
    for (int bit = 7; bit >= 0; bit--) {
        lcd_writebit((0x20 >> bit) & 0x01);
    }
    _delay_ms(2);
    
    // Configure the display
    lcd_writecommand(0x20);    // Function set: 8-bit interface, 1 line
    lcd_writecommand(0x0C);    // Display on, cursor off, blink off
    lcd_writecommand(0x01);    // Clear display
    _delay_ms(2);              // Clear display command needs > 1.5ms
    lcd_writecommand(0x06);    // Entry mode: increment cursor, no shift
}

/*
  lcd_moveto - Move the cursor to the row and column given by the arguments.
  Row is 0 or 1, column is 0 - 15.
*/
void lcd_moveto(unsigned char row, unsigned char col)
{
    unsigned char pos;
    if(row == 0) {
        pos = 0x80 + col;       // 1st row locations start at 0x80
    }
    else {
        pos = 0xc0 + col;       // 2nd row locations start at 0xc0
    }
    lcd_writecommand(pos);      // Send command
}

/*
  lcd_stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void lcd_stringout(char *str)
{
    int i = 0;
    while (str[i] != '\0') {    // Loop until next character is NULL byte
        lcd_writedata(str[i]);  // Send the character
        i++;
    }
}

/*
  lcd_writecommand - Output a byte to the LCD command register using serial communication
*/
void lcd_writecommand(unsigned char cmd)
{
    /* Clear RS pin (PB0) to 0 for a command transfer */
    PORTB &= ~RS_PIN;
    
    /* Send each bit of the command byte serially */
    for (int bit = 7; bit >= 0; bit--) {
        lcd_writebit((cmd >> bit) & 0x01);
    }
    
    /* Delay 2ms */
    _delay_ms(2);
}

/*
  lcd_writedata - Output a byte to the LCD data register using serial communication
*/
void lcd_writedata(unsigned char dat)
{
    /* Set RS pin (PB0) to 1 for a data transfer */
    PORTB |= RS_PIN;
    
    /* Send each bit of the data byte serially */
    for (int bit = 7; bit >= 0; bit--) {
        lcd_writebit((dat >> bit) & 0x01);
    }
    
    /* Delay 2ms */
    _delay_ms(2);
}

/*
  lcd_writebit - Output a single bit to the LCD
*/
void lcd_writebit(unsigned char bit)
{
    /* Set or clear the data pin based on the bit value */
    if (bit) {
        PORTD |= DATA_PIN;    // Set data pin high
    } else {
        PORTD &= ~DATA_PIN;   // Set data pin low
    }
    
    /* Toggle the enable pin to clock in the bit */
    PORTD |= ENABLE_PIN;       // Set enable pin high
    _delay_us(1);              // Small delay
    PORTD &= ~ENABLE_PIN;      // Set enable pin low
    _delay_us(1);              // Small delay between bits
}