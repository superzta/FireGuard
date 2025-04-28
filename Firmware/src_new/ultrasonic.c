#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "ultrasonic.h"

#ifndef F_CPU
#define F_CPU 7372800UL 
#endif

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Ultrasonic sensor pins (PD6 for trigger, PD7 for echo)
#define TRIG_PIN PD6
#define ECHO_PIN PD7

// Constants for distance calculation
#define SOUND_SPEED 0.0343    // Speed of sound in cm/microsecond
#define MAX_DISTANCE 400.0    // Maximum valid distance in cm

// Global variables
volatile uint16_t pulse_width = 0;    // Echo pulse width in timer ticks
volatile uint8_t echo_complete = 0;   // Flag to indicate measurement complete
volatile uint8_t timeout = 0;         // Flag to indicate timeout (out of range)

// Function prototypes
void uart_init(uint16_t ubrr);
void uart_transmit(uint8_t data);
void uart_print_string(char* str);
void ultrasonic_init(void);
void timer1_init(void);
void pin_change_init(void);
float calculate_distance(uint16_t pulse_width);
void trigger_measurement(void);

// Timer1 overflow interrupt - handles timeout
ISR(TIMER1_OVF_vect) {
    // Stop timer
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
    timeout = 1;
    echo_complete = 1;
}

// Pin change interrupt for echo pin
ISR(PCINT2_vect) {
    // Rising edge - start timer
    if (PIND & (1 << ECHO_PIN)) {
        // Reset timer counter
        TCNT1 = 0;
        // Start timer with prescaler of 8
        TCCR1B |= (1 << CS11);
        timeout = 0;
    } 
    // Falling edge - stop timer and calculate pulse width
    else if (!(PIND & (1 << ECHO_PIN)) && !timeout) {
        // Stop timer
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        // Record pulse width
        pulse_width = TCNT1;
        echo_complete = 1;
    }
}




// Initialize UART communication
void uart_init(uint16_t ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    
    // Enable transmitter
    UCSR0B = (1 << TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (3 << UCSZ00);
}

// Transmit single byte via UART
void uart_transmit(uint8_t data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Put data into buffer, sends the data
    UDR0 = data;
}

// Transmit string via UART
void uart_print_string(char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

// Initialize ultrasonic sensor pins
void ultrasonic_init(void) {
    // Set trigger pin as output
    DDRD |= (1 << TRIG_PIN);
    
    // Set echo pin as input
    DDRD &= ~(1 << ECHO_PIN);
    
    // Initialize timer and interrupts
    timer1_init();
    pin_change_init();
    
    // Enable global interrupts if not already enabled
    sei();
}

// Initialize Timer1 for pulse width measurement
void timer1_init(void) {
    // Normal mode
    TCCR1A = 0;
    TCCR1B = 0;
    
    // Enable overflow interrupt
    TIMSK1 |= (1 << TOIE1);
}

// Initialize pin change interrupt for echo pin
void pin_change_init(void) {
    // Enable pin change interrupt for PCINT23 (PD7)
    PCICR |= (1 << PCIE2);  // Enable pin change interrupt for PCINT[23:16]
    PCMSK2 |= (1 << PCINT23); // Enable pin change detection for PCINT23 (PD7)
}

// Calculate distance in cm from pulse width
float calculate_distance(uint16_t pulse_width) {
    // Timer1 with prescaler 8: each count is 8/F_CPU seconds
    // Convert pulse_width to microseconds
    float pulse_microseconds = (float)pulse_width * 8.0 * 1000000.0 / F_CPU;
    
    // Calculate distance: distance = (time * speed of sound) / 2
    float distance = (pulse_microseconds * SOUND_SPEED) / 2.0;
    
    // Limit to maximum range
    if (distance > MAX_DISTANCE) {
        distance = MAX_DISTANCE;
    }
    
    return distance;
}

// Trigger a new distance measurement
void trigger_measurement(void) {
    // Clear trigger pin
    PORTD &= ~(1 << TRIG_PIN);
    _delay_us(2);
    
    // Send 10us pulse to trigger pin
    PORTD |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1 << TRIG_PIN);
}

// Measure distance with ultrasonic sensor
float measure_distance(void) {
    // Reset flags
    echo_complete = 0;
    timeout = 0;
    
    // Trigger a new measurement
    trigger_measurement();
    
    // Wait for measurement to complete (with timeout)
    uint16_t timeout_counter = 0;
    while (!echo_complete && timeout_counter < 30000) {
        _delay_us(1);
        timeout_counter++;
    }
    
    // Calculate distance based on pulse width
    if (timeout || timeout_counter >= 30000) {
        return MAX_DISTANCE;
    } else {
        return calculate_distance(pulse_width);
    }
}


// Exclude main function when compiling as a library
#ifndef EXCLUDE_MAIN
int main(void) {
    // Initialize UART
    uart_init(MYUBRR);
    
    // Initialize ultrasonic sensor and timer
    ultrasonic_init();
    timer1_init();
    pin_change_init();
    
    // Enable global interrupts
    sei();
    
    // Variables for distance measurement
    float distance;
    char buffer[20];
    
    _delay_ms(500); // Startup delay
    
    uart_print_string("Ultrasonic Sensor Started\r\n");
    
    while (1) {
        // Reset flags
        echo_complete = 0;
        timeout = 0;
        
        // Trigger a new measurement
        trigger_measurement();
        
        // Wait for measurement to complete
        while (!echo_complete) {
            // Wait for echo or timeout
        }
        
        // Calculate distance based on pulse width
        if (timeout) {
            distance = MAX_DISTANCE;
            uart_print_string("Out of range\r\n");
        } else {
            distance = calculate_distance(pulse_width);
            
            // Convert float to string and send over UART
            dtostrf(distance, 6, 2, buffer);
            uart_print_string("Distance: ");
            uart_print_string(buffer);
            uart_print_string(" cm\r\n");
        }
        
        _delay_ms(200);  // Wait between measurements
    }
    
    return 0;
}

#endif