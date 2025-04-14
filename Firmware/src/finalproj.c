/********************************************
 *
 *  Name: Owen Zeng
 *  Email: owenzeng@usc.edu
 *  Section: W 3:30-5
 *  Assignment: Final Project
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "finalproj.h"
#include "lcd.h"

#define ThresAddLocal 10
#define ThresAddRemote 20
#define FOSC 16000000 // Clock frequency
#define BAUD 9600 // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0

int main(void)
{
	// Initializations
	lcd_init();
    adc_init();
    timer0_init();
	timer1_init();	
    timer2_init();

    DDRC &= ~(1 << PC0); // Make sure PC0 is an input (Right button on LCD)
	DDRC |= (1 << PC1); // Set PC1 as the TRIG output
    DDRB |= (1 << PB3); // Set PB3 as the Servo PWM output
    DDRC |= (1 << PC4); // Set PC4 as the RGB output
    DDRB |= (1 << PB5); // Set PB5 as the RGB output
    DDRC |= (1 << PC3); // Set PC3 as tristate enable (active low) output
    DDRC |= (1 << PC5); // Set PC5 as output for Buzzer

	PORTB |= (1 << PB4); // Enable pull-up resistor for acquire button
    PORTD |= (1 << PD2); 
    PORTD |= (1 << PD3); // Enable pull-up resistor for the rotary encoder
    PORTC &= ~(1 << PC3); // Set tristate enable to low
    
    // Rx and Tx init
    UBRR0 = MYUBRR; // Set baud rate
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0); // Enable RX and TX
    UCSR0B |= (1 << RXCIE0);    // Enable RX interrupt
    UCSR0C = (3 << UCSZ00); // Async., no parity, 1 stop bit, 8 data bits

    // Write splash screen
	lcd_writecommand(1); // Clear the display
    lcd_writecommand(0x0C);     // Display and cursor off
    lcd_moveto(0,3);    // Move to the top left corner
    // Print the splash screen
    lcd_stringout("EE109 Project"); 
    lcd_moveto(1,3);    // Move to the bottom row
    lcd_stringout("Owen Zeng");
    //lcd_writecommand(2); // Move cursor to home
    _delay_ms(500);    // Delay 0.5 second
    lcd_writecommand(1); // Clear the display

	PCICR |= (1 << PCIE1); // Enable Pin Change Interrupt for group C
    PCICR |= (1 << PCIE2); // Enable Pin Change Interrupt for group D
	PCMSK1 |= (1 << PCINT10); // Enable Pin Change Interrupt for PC2

    Threshold_Local = eeprom_read_word((void *)ThresAddLocal);  // Read the local threshold from EEPROM
    if (Threshold_Local < 1 || Threshold_Local > 400) { // Check if the threshold is valid
        Threshold_Local = 10;
    }
    Threshold_Remote = eeprom_read_word((void *)ThresAddRemote);    // Read the remote threshold from EEPROM
    if (Threshold_Remote < 1 || Threshold_Remote > 400) {   // Check if the threshold is valid
        Threshold_Remote = 10;
    }

    PCMSK2 |= (1 << PCINT18); // Enable Pin Change Interrupt for PD2 
    PCMSK2 |= (1 << PCINT19); // and PD3

	sei(); // Enable global interrupts

    uint8_t state = ADJUST;    // Set initial state to ADJUST
    uint8_t thres_setting = LOCAL;     // Set initial adjust to LOCAL

    lcd_moveto(0,0);    // Move to the top left corner
    lcd_stringout("Local"); // Print initial threshold setting
    lcd_moveto(1,0);    // Move to the bottom left corner
    char TBuff[16];
    sprintf(TBuff, "%3d", Threshold_Local); // Print distance with one decimal place
    lcd_stringout("Min= ");
    lcd_stringout(TBuff);
    RotCount = Threshold_Local;

    RGB (BLUE); // Set RGB default to blue

 while(1){
    bntsts = adc_sample(0);   // Read button status
    PCMSK2 |= (1 << PCINT18); // Enable Pin Change Interrupt for PD2 
    PCMSK2 |= (1 << PCINT19); // and PD3
    if (state == ACQUIRE){
	    // if the acquire button is pressed, start the ultrasonic sensor
	    if (!(PINB & (1 << PB4))) {
		    // set TRIG to high
		    debounce(PB4);
            while (!(PINB & (1 << PB4))) {
            }
		    PORTC &= ~(1 << PC1);
		    _delay_us(2);
		    PORTC |= (1 << PC1);
		    _delay_us(10);
		    PORTC &= ~(1 << PC1);
		    debounce(PB4);
        }
        lcd_writecommand(0x0C);     // Display on cursor off
        if (change && !timeout){    
		    change = 0;
            distance_x10 = pulse_count * 34 /100 / 4;
            OCR2A = distance_x10 * -575 / 100000 + 35; // Map distance to servo position
            lcd_moveto(0,10);    // Move to the top row
            lcd_stringout("      "); // clear the distance or >400.0
            lcd_moveto(0,11);    // Move to the top row
            char buffer[16];
            sprintf(buffer, "%3ld.%ld", distance_x10 / 10, distance_x10 % 10); // Print distance with one decimal place
            lcd_stringout(buffer);
            //lcd_stringout(" cm");
            char txbuffer[16];
            sprintf(txbuffer, "%ld", distance_x10); // Print distance with one decimal place
            tx_char('<'); // Send start character
            tx_string(txbuffer); // Send end character
            tx_char('>'); // Send end character
            if (distance_x10/10 < Threshold_Local){ // Check if the distance is less than the threshold
                RGB (RED);
            }else{
                RGB (GREEN); 
            }
	    }else if (change && timeout){
            change = 0;
            lcd_moveto(0,10);    
            lcd_stringout("      ");
            lcd_moveto(0,10);    
            lcd_stringout(">400.0");
            timeout = 0;
            distance_x10 = 4000;
            RGB (BLUE); // Measurement failed, set RGB to blue
        }

        if (bntsts >=0 && bntsts < 10) { // if right button pressed
        state = ADJUST;
        AdjUpdate = 1;
        }
        
    }
    if (state == ADJUST){
        debounce(PC0);
        if(bntsts >=0 && bntsts < 10){
            while (bntsts >=0 && bntsts < 10){  // wait for button release
            bntsts = adc_sample(0);   // Read button status
            AdjUpdate = 1;
            } 
        }

        lcd_writecommand(0x0F);     // Display and cursor blinking
        if (thres_setting == LOCAL && AdjUpdate){    // Remote Setting
            thres_setting = REMOTE;
            // Print remote threshold setting
            RotCount = Threshold_Remote;
            lcd_moveto(0,0);   
            lcd_stringout("      ");
            lcd_moveto(0,0);    
            lcd_stringout("Remote"); 
            lcd_moveto(1,0);    
            lcd_stringout("        ");
            lcd_moveto(1,0);
            lcd_stringout("Min= ");
            char MinBufRMT[16];
            sprintf(MinBufRMT, "%3d", Threshold_Remote); // Print distance with one decimal place
            lcd_stringout(MinBufRMT);
            AdjUpdate = 0;

        }else if (thres_setting == REMOTE && AdjUpdate){  // Local Setting
            thres_setting = LOCAL;
            // Print local threshold setting
            RotCount = Threshold_Local;
            lcd_moveto(0,0);
            lcd_stringout("      ");
            lcd_moveto(0,0);
            lcd_stringout("Local");
            lcd_moveto(1,0);
            lcd_stringout("        ");
            lcd_moveto(1,0);
            lcd_stringout("Min= ");
            char MinBuf[16];
            sprintf(MinBuf, "%3d", Threshold_Local); // Print distance with one decimal place
            lcd_stringout(MinBuf);
            AdjUpdate = 0;
        }
        debounce(PC0);  
        if (!(PINB & (1 << PB4))) { // if the acquire button is pressed, start the ultrasonic sensor
            state = ACQUIRE;
        }
    }

    if (rxfinish){  // if receive data from remote
        rxfinish = 0;
        sscanf(rxbuf, "%ld", &distance_x10_remote); // Convert string to long
        lcd_moveto(1,11); // Move to the bottom row
        lcd_stringout("      ");
        lcd_moveto(1,11); 
        char bufrxf[16];
        sprintf(bufrxf, "%3ld.%ld", distance_x10_remote / 10, distance_x10_remote % 10); // Print distance with one decimal place
        lcd_stringout(bufrxf);
        if (thres_setting == LOCAL){ // Local Setting
            if (distance_x10_remote/10 < Threshold_Local){
                sound_buzz();
            }
        }else{
            if (distance_x10_remote/10 < Threshold_Remote){
                sound_buzz();
            }
        }
    }

    if(RotChange ){ // if only rotary encoder is rotated
        state = ADJUST; // Change to adjust state
        AdjUpdate = 0;  // No need to update the threshold
        RotChange = 0; 
        if (thres_setting == LOCAL){    // Local Setting
            eeprom_update_word((void *)ThresAddLocal, RotCount);    // Update the threshold stored in EEPORM from rotary encoder
            Threshold_Local = eeprom_read_word((void *)ThresAddLocal);  // Read the threshold stored in EEPORM
            lcd_moveto(1,0);    
            lcd_stringout("        ");
            lcd_moveto(1,0);
            lcd_stringout("Min= ");
            char MinBuf[16];
            sprintf(MinBuf, "%3d", Threshold_Local); // Print distance with one decimal place
            lcd_stringout(MinBuf);
        }else{  // Remote Setting
            eeprom_update_word((void *)ThresAddRemote, RotCount);
            Threshold_Remote = eeprom_read_word((void *)ThresAddRemote);
            lcd_moveto(1,0);    
            lcd_stringout("        ");
            lcd_moveto(1,0);
            lcd_stringout("Min= ");
            char MBufRMT[16];
            sprintf(MBufRMT, "%3d", Threshold_Remote); // Print distance with one decimal place
            lcd_stringout(MBufRMT);
        }
    }
 }
    return 0;   /* never reached */
}

void debounce(uint8_t bit)  
{
	_delay_ms(5);
}

