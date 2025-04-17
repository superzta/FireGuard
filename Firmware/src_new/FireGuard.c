#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

#include "I2C.h"
#include "stepper.h"
#include "servo.h"

#ifndef F_CPU
#define F_CPU 7372800UL
#endif

#define BAUD_RATE 230400

// Scanning motion parameters - easily changeable
#define SCAN_RANGE_STEPS 800   // 120 degrees of motion (approximately)
#define STEPS_PER_CHECK 20     // Check temperature every 20 steps
#define MOTOR_STEP_DELAY 10    // Milliseconds between steps to control speed

// Threshold temperature for fire detection (in centidegrees)
#define FIRE_THRESHOLD 5000  // 50.00°C

// Position constraints for fire confirmation (helps prevent false positives)
#define FIRE_COL_MIN 0
#define FIRE_COL_MAX 3

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
    
    // Start with counter-clockwise direction (default)
    set_stepper_direction(false);

    // Initialize servo
    servo_init();
    
    // Main variables
    uint16_t current_step = 0;
    bool scanning_forward = false;  // Start counter-clockwise
    bool fire_detected = false;
    
    serial_println("Starting fire detection patrol with scanning motion...");
    
    // Main loop
    while (1) {
        //patrol until fire detected
        while (!fire_detected) {
            // Step the motor once in current direction
            move_bottom_stepper_once();
            _delay_ms(MOTOR_STEP_DELAY);
            
            current_step++;
            
            // Check if we need to reverse direction
            if (current_step >= SCAN_RANGE_STEPS) {
                // Change direction
                scanning_forward = !scanning_forward;
                set_stepper_direction(scanning_forward);
                
                // Reset step counter
                current_step = 0;
                
                // Log direction change
                if (scanning_forward) {
                    serial_println("Changing direction: Clockwise");
                } else {
                    serial_println("Changing direction: Counter-clockwise");
                }
            }
            
            // Check temperature periodically
            if (current_step % STEPS_PER_CHECK == 0) {
                // Read thermal data from sensor
                result = mlx90640_read_center_region();
                
                // Process the reading if successful
                if (result == 0) {
                    // Print status update
                    char buffer[64];
                    int16_t int_part = max_temp / 100;
                    uint8_t frac_part = abs(max_temp) % 100;
                    
                    sprintf(buffer, "Position: %d/%d | Max: %d.%02d°C at [%d][%d]", 
                            current_step, SCAN_RANGE_STEPS, int_part, frac_part, 
                            max_row_pos, max_col_pos);
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
        
        // Keep monitoring in alert mode if fire is detected
        while (fire_detected) {
            result = mlx90640_read_center_region();
            
            if (result == 0) {
                char buffer[64];
                int16_t int_part = max_temp / 100;
                uint8_t frac_part = abs(max_temp) % 100;
                
                sprintf(buffer, "Alert! Current temp: %d.%02d°C at [%d][%d]", 
                        int_part, frac_part, max_row_pos, max_col_pos);
                serial_println(buffer);

                // Set servo to 0 and 105 degrees
                set_servo_degree(0);
                _delay_ms(1000);
                set_servo_degree(105);
                _delay_ms(1000);

            }

            // Check if fire is detected
            if (max_temp > FIRE_THRESHOLD && 
                max_col_pos >= FIRE_COL_MIN && max_col_pos <= FIRE_COL_MAX) {
                fire_detected = true;
            } else {
                fire_detected = false;
                serial_println("Fire alert mode ended");
            }
            
            // Update at 1Hz in alert mode
            _delay_ms(1000);
        }
    }
    
    return 0;
}
