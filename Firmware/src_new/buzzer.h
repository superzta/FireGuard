#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

// Function prototypes
void buzzer_init(void);
void buzzer_sound(uint16_t duration_ms);
void buzzer_silent(uint16_t duration_ms);
void buzzer_warning(void);

#endif // BUZZER_H 