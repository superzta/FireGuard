#include <avr/io.h>
#include <util/delay.h>

#define SERVO_MIN_PULSE 750    // in microseconds
#define SERVO_MAX_PULSE 2250   // in microseconds
#define PWM_PERIOD_US    16000 // 16ms typical servo period

#define PRESCALER 64
#define F_CPU 7372800UL
#define TIMER_TICK_US (PRESCALER * 1.0 / (F_CPU / 1000000UL))  // 4us per tick

#define SERVO_MIN_OCR (SERVO_MIN_PULSE / TIMER_TICK_US)  // = 750 / 4 = ~188
#define SERVO_MAX_OCR (SERVO_MAX_PULSE / TIMER_TICK_US)  // = 2250 / 4 = ~563

void servo_init() {
    DDRD |= (1 << PD3); // OC2B output

    // Fast PWM mode, TOP = 255, non-inverting mode on OC2B
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << CS22); // Prescaler = 64

    // set initial position to left
    OCR2B = SERVO_MIN_OCR;  

    // // set initial position to right
    // OCR2B = SERVO_MAX_OCR;
}

void set_servo_degree(uint8_t degree) {
    if (degree > 180) degree = 180;
    uint16_t ocr_value = SERVO_MIN_OCR + (degree * (SERVO_MAX_OCR - SERVO_MIN_OCR)) / 180;
    OCR2B = ocr_value;
}

#ifndef EXCLUDE_MAIN
int main() {
    servo_init();
    _delay_ms(3000);
    
    while (1) {
        set_servo_degree(0);   // full left
        _delay_ms(2000);
        set_servo_degree(105);
        _delay_ms(2000);
    }
}
#endif
