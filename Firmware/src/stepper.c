#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h> // Add this for bool type support

// Include our header file
#include "stepper.h"

// EasyDriver pin mappings
#define STEP_PIN_BTM    PC1
#define STEP_PIN_TOP    PC2

// The two direction pins are shorted together and connected to ground so that the motor driver is always on for both directions
#define DIR_PIN     PC3
// Enable pins are shorted together and connected to ground so that the motor driver is always on for both directions

// Remove the MS1 and MS2 pin definitions since they're hardwired
// #define MS1_PIN     PD5
// #define MS2_PIN     PD6

// Define delay as a constant for _delay_us to work properly
#define STEP_DELAY_US 2000

void setup_pins() {
    // Configure only the pins we control in software as outputs
    DDRC |= (1 << STEP_PIN_BTM) | (1 << STEP_PIN_TOP) | (1 << DIR_PIN);
}

void set_stepper_direction(bool move_clockwise) {
    if (move_clockwise) {
        PORTC |= (1 << DIR_PIN);
    } else {
        PORTC &= ~(1 << DIR_PIN);
    }
}

void move_top_stepper(bool top_step) {
    if (top_step) {
        PORTC |= (1 << STEP_PIN_TOP);
        _delay_us(STEP_DELAY_US);
        PORTC &= ~(1 << STEP_PIN_TOP);
        _delay_us(STEP_DELAY_US);
    } else {
        PORTC |= (1 << STEP_PIN_BTM);
        _delay_us(STEP_DELAY_US);
        PORTC &= ~(1 << STEP_PIN_BTM);
        _delay_us(STEP_DELAY_US);
    }
}

void move_bottom_stepper_once() {
    PORTC |= (1 << STEP_PIN_BTM);   // Step HIGH
    _delay_us(STEP_DELAY_US);
    PORTC &= ~(1 << STEP_PIN_BTM);  // Step LOW
    _delay_us(STEP_DELAY_US);
}

// Main function - only include when not compiled as a library
#ifndef EXCLUDE_MAIN
int main(void) {
    setup_pins();

    // delay for 10 seconds
    _delay_ms(5000);
    
    // Set direction to forward once
    PORTC |= (1 << DIR_PIN);
    
    int step_count = 0;
    while (1) {
        move_top_stepper(false);
        // delay for 100us
        _delay_us(10000);
        step_count++;

        if (step_count > 2500) {
            PORTC |= (1 << STEP_PIN_TOP);
            break;
        }
    }
}
#endif // EXCLUDE_MAIN