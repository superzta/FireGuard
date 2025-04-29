#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "buzzer.h"

// Buzzer connected to PD5
#define BUZZER_PIN PD5
#define BUZZER_PORT PORTD
#define BUZZER_DDR DDRD

// Buzzer frequency (Hz)
#define BUZZER_FREQ 5000
// Time period for one cycle in microseconds
#define CYCLE_PERIOD_US (1000000 / BUZZER_FREQ)
// Half cycle time in microseconds (for 50% duty cycle)
#define HALF_CYCLE_US (CYCLE_PERIOD_US / 2)

// Timing for buzz pattern
#define BUZZ_DURATION_MS 50  // 1 second of sound
#define SILENT_DURATION_MS 50  // 5 seconds of silence

// Function to initialize buzzer pin
void buzzer_init(void) {
    // Set buzzer pin as output
    BUZZER_DDR |= (1 << BUZZER_PIN);
    // Start with buzzer off
    BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

// Function to generate square wave for given duration
void buzzer_sound(uint16_t duration_ms) {
    uint16_t cycles = (duration_ms * 1000) / CYCLE_PERIOD_US;
    
    for (uint16_t i = 0; i < cycles; i++) {
        // High for half cycle
        BUZZER_PORT |= (1 << BUZZER_PIN);
        _delay_us(HALF_CYCLE_US);
        
        // Low for half cycle
        BUZZER_PORT &= ~(1 << BUZZER_PIN);
        _delay_us(HALF_CYCLE_US);
    }
}

// Function to wait silently
void buzzer_silent(uint16_t duration_ms) {
    // Ensure buzzer is off
    BUZZER_PORT &= ~(1 << BUZZER_PIN);
    
    // Fixed delay loop instead of variable delay
    for (uint16_t i = 0; i < duration_ms / 10; i++) {
        _delay_ms(10);  // Fixed 10ms delay increments
    }
    
    // Handle remaining time less than 10ms
    uint8_t remainder = duration_ms % 10;
    if (remainder > 0) {
        if (remainder == 1) _delay_ms(1);
        else if (remainder == 2) _delay_ms(2);
        else if (remainder == 3) _delay_ms(3);
        else if (remainder == 4) _delay_ms(4);
        else if (remainder == 5) _delay_ms(5);
        else if (remainder == 6) _delay_ms(6);
        else if (remainder == 7) _delay_ms(7);
        else if (remainder == 8) _delay_ms(8);
        else if (remainder == 9) _delay_ms(9);
    }
}

void buzzer_warning(void) {
    for (int i = 0; i < 12; i++) {
        buzzer_sound(BUZZ_DURATION_MS);
        buzzer_silent(SILENT_DURATION_MS);
        _delay_us(10);
    }
}