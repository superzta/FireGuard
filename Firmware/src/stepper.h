#ifndef STEPPER_H
#define STEPPER_H

#include <stdbool.h>

// EasyDriver pin mappings
#define STEP_PIN_BTM    PC1
#define STEP_PIN_TOP    PC2
#define DIR_PIN         PC3
#define STEP_DELAY_US   2000

// Stepper motor functions
void setup_pins(void);
void set_stepper_direction(bool move_clockwise);
void move_top_stepper(bool top_step);
void move_bottom_stepper_once(void);

#endif /* STEPPER_H */ 