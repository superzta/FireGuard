#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

#include "I2C.h"
#include "stepper.h"

#ifndef F_CPU
#define F_CPU 7372800UL
#endif

#define BAUD_RATE 230400

// Threshold temperature for fire detection (in centidegrees)
#define FIRE_THRESHOLD 5000  // 50.00°C

// Position constraints for fire confirmation (helps prevent false positives)
#define FIRE_COL_MIN 0
#define FIRE_COL_MAX 3

// Steps between temperature checks
#define STEPS_PER_CHECK 20

int main(void) {
    // Disable watchdog
    MCUSR = 0;
    wdt_disable();
    
    // Initialize UART
    serial_init((F_CPU / 16 / BAUD_RATE) - 1);
    
    // Send welcome message
    serial_println("FireGuard System Initializing...");
    
    // Initialize thermal sensor
    i2c_init();
    _delay_ms(100);
    
    int result = mlx90640_init();
    if (result != 0) {
        char buffer[32];
        sprintf(buffer, "Thermal sensor init failed: %d", result);
        serial_println(buffer);
    } else {
        serial_println("Thermal sensor initialized successfully");
    }
    
    // Initialize stepper motor
    setup_pins();
    set_stepper_direction(true);  // Set direction clockwise
    
    // Main variables
    uint16_t steps = 0;
    bool fire_detected = false;
    
    serial_println("Starting fire detection patrol...");
    
    // Main loop - patrol until fire detected
    while (!fire_detected) {
        // Step the bottom motor once (using stepper functions exactly as in stepper.c)
        move_bottom_stepper_once();
        _delay_ms(10);  // Short delay to slow down motor movement
        
        steps++;
        
        // Check temperature periodically
        if (steps >= STEPS_PER_CHECK) {
            steps = 0;
            
            // Read thermal data from sensor
            result = mlx90640_read_center_region();
            
            // Process the reading if successful
            if (result == 0) {
                // Print status update
                char buffer[48];
                int16_t int_part = max_temp / 100;
                uint8_t frac_part = abs(max_temp) % 100;
                
                sprintf(buffer, "Max: %d.%02d°C at [%d][%d]", 
                        int_part, frac_part, max_row_pos, max_col_pos);
                serial_println(buffer);
                
                // Check if fire detected (temp > threshold and in target columns)
                if (max_temp > FIRE_THRESHOLD && 
                    max_col_pos >= FIRE_COL_MIN && max_col_pos <= FIRE_COL_MAX) {
                    
                    fire_detected = true;
                    
                    // Detailed fire detection message
                    char alert_buffer[64];
                    sprintf(alert_buffer, "FIRE DETECTED! Temp: %d.%02d°C at position [%d][%d]", 
                            int_part, frac_part, max_row_pos, max_col_pos);
                    serial_println(alert_buffer);
                }
            } else {
                serial_println("Error reading thermal data");
            }
        }
    }
    
    // Fire alert mode - motor stopped, monitoring continues
    serial_println("Motor stopped - FIRE ALERT MODE");
    
    // Keep monitoring in alert mode
    while (1) {
        result = mlx90640_read_center_region();
        
        if (result == 0) {
            char buffer[64];
            int16_t int_part = max_temp / 100;
            uint8_t frac_part = abs(max_temp) % 100;
            
            sprintf(buffer, "Alert! Current temp: %d.%02d°C at [%d][%d]", 
                    int_part, frac_part, max_row_pos, max_col_pos);
            serial_println(buffer);
        }
        
        // Update at 1Hz in alert mode
        _delay_ms(1000);
    }
    
    return 0;
}
