#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>


#define SERVO_MIN_PULSE 750    // in microseconds
#define SERVO_MAX_PULSE 2250   // in microseconds
#define PWM_PERIOD_US    16000 // 16ms typical servo period

#define PRESCALER 64
#define F_CPU 7372800UL
#define TIMER_TICK_US (PRESCALER * 1.0 / (F_CPU / 1000000UL))  // 4us per tick

#define SERVO_MIN_OCR (SERVO_MIN_PULSE / TIMER_TICK_US)  // = 750 / 4 = ~188
#define SERVO_MAX_OCR (SERVO_MAX_PULSE / TIMER_TICK_US)  // = 2250 / 4 = ~563

// Stepper motor functions
void servo_init();
void set_servo_degree(uint8_t degree);

#endif /* SERVO_H */ 