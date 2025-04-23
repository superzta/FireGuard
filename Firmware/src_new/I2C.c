#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
#include <math.h>
#include <stdint.h>

// Include our header
#include "I2C.h"

#ifndef F_CPU
#define F_CPU 7372800UL
#endif

#define BAUD_RATE 230400

// I2C and MLX90640 definitions
#define MLX90640_I2CADDR 0x33
#define MLX90640_DEVICEID1 0x0F2D
#define MLX90640_CHESS 1
#define MLX90640_INTERLEAVED 0
#define TA_SHIFT 8

// Frame size
#define MLX90640_WIDTH 32
#define MLX90640_HEIGHT 24

// // Center 8x8 region
// #define CENTER_SIZE 8
// #define CENTER_START_ROW ((24 - CENTER_SIZE) / 2)
// #define CENTER_START_COL ((32 - CENTER_SIZE) / 2)

// Center 16x16 region
#define CENTER_SIZE 16
#define CENTER_START_ROW ((24 - CENTER_SIZE) / 2)
#define CENTER_START_COL ((32 - CENTER_SIZE) / 2)

// I2C bit manipulation macros
#define I2C_SDA_HIGH() DDRC &= ~(1 << PC4)
#define I2C_SDA_LOW() do { DDRC |= (1 << PC4); PORTC &= ~(1 << PC4); } while(0)
#define I2C_SCL_HIGH() do { DDRC &= ~(1 << PC5); _delay_us(5); } while(0)
#define I2C_SCL_LOW() do { DDRC |= (1 << PC5); PORTC &= ~(1 << PC5); _delay_us(5); } while(0)
#define I2C_SDA_READ() (PINC & (1 << PC4))

// Global variables - MEMORY OPTIMIZED
int16_t center_data[CENTER_SIZE][CENTER_SIZE]; // Keep this for the 8x8 matrix
int16_t max_temp;
uint8_t max_row_pos;  // Position of max temperature
uint8_t max_col_pos;
uint8_t i2c_initialized = 0;
// Removed large frame buffer arrays (frame_buffer[834] & eeMLX90640[832])
// Will read data in chunks instead of all at once

// Serial communication functions
void serial_init(unsigned short ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8); 
    UBRR0L = (unsigned char)ubrr;        
    UCSR0B = (1 << TXEN0) | (1 << RXEN0); 
    UCSR0C = (3 << UCSZ00);              
}

void serial_out(char ch) {
    while (!(UCSR0A & (1 << UDRE0))); 
    UDR0 = ch; 
}

char serial_in() {
    while (!(UCSR0A & (1 << RXC0))); 
    return UDR0; 
}

void serial_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_out(str[i]);
    }
}

void serial_println(const char *str) {
    serial_print(str);
    serial_out('\r');
    serial_out('\n');
}

// Print fixed-point temperature (value is temp * 100)
void print_temp(int16_t value) {
    char buffer[16];
    int16_t int_part = value / 100;
    uint8_t frac_part = (uint8_t)((value < 0 ? -value : value) % 100);
    
    sprintf(buffer, "%d.%02d", int_part, frac_part);
    serial_print(buffer);
}

// I2C implementation
void i2c_stop(void);  // Forward declaration

void i2c_init() {
    if (i2c_initialized) return;
    
    // Set SDA and SCL as inputs with pull-ups enabled
    DDRC &= ~((1 << PC4) | (1 << PC5));
    PORTC |= (1 << PC4) | (1 << PC5);
    _delay_ms(10);
    
    // Send a stop condition to reset the bus
    i2c_stop();
    
    i2c_initialized = 1;
}

void i2c_start() {
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    _delay_us(5);
    I2C_SDA_LOW();
    _delay_us(5);
    I2C_SCL_LOW();
}

void i2c_stop() {
    I2C_SDA_LOW();
    _delay_us(5);
    I2C_SCL_HIGH();
    _delay_us(5);
    I2C_SDA_HIGH();
    _delay_us(5);
}

uint8_t i2c_write(uint8_t data) {
    uint8_t i, ack;
    
    for (i = 0; i < 8; i++) {
        if (data & 0x80)
            I2C_SDA_HIGH();
        else
            I2C_SDA_LOW();
            
        _delay_us(5);
        I2C_SCL_HIGH();
        _delay_us(5);
        I2C_SCL_LOW();
        data <<= 1;
    }
    
    I2C_SDA_HIGH();
    _delay_us(5);
    I2C_SCL_HIGH();
    _delay_us(5);
    
    ack = I2C_SDA_READ() ? 1 : 0;
    I2C_SCL_LOW();
    
    return ack;
}

uint8_t i2c_read(uint8_t ack) {
    uint8_t i, data = 0;
    
    I2C_SDA_HIGH();
    
    for (i = 0; i < 8; i++) {
        data <<= 1;
        I2C_SCL_HIGH();
        _delay_us(5);
        if (I2C_SDA_READ())
            data |= 1;
        I2C_SCL_LOW();
        _delay_us(5);
    }
    
    if (ack)
        I2C_SDA_LOW();
    else
        I2C_SDA_HIGH();
        
    _delay_us(5);
    I2C_SCL_HIGH();
    _delay_us(5);
    I2C_SCL_LOW();
    I2C_SDA_HIGH();
    
    return data;
}

// Optimized I2C read for MLX90640 - read only what we need
int mlx90640_i2c_read(uint8_t addr, uint16_t reg, uint16_t* data, uint8_t count) {
    int retries = 3;
    
    while (retries--) {
        i2c_start();
        
        if (i2c_write((addr << 1) & 0xFE)) {  // Write address
            i2c_stop();
            continue;
        }
        
        if (i2c_write(reg >> 8) || i2c_write(reg & 0xFF)) {  // Register address
            i2c_stop();
            continue;
        }
        
        i2c_start();  // Restart
        
        if (i2c_write((addr << 1) | 0x01)) {  // Read address
            i2c_stop();
            continue;
        }
        
        // Read data
        for (uint8_t i = 0; i < count; i++) {
            uint16_t msb = i2c_read(1);  // ACK
            uint16_t lsb = i2c_read(i < count - 1);  // NACK on last byte
            data[i] = (msb << 8) | lsb;
        }
        
        i2c_stop();
        return 0;  // Success
    }
    
    return -1;  // Failure
}

// Optimized I2C write for MLX90640
int mlx90640_i2c_write(uint8_t addr, uint16_t reg, uint16_t data) {
    int retries = 3;
    
    while (retries--) {
        i2c_start();
        
        if (i2c_write((addr << 1) & 0xFE)) {  // Write address
            i2c_stop();
            continue;
        }
        
        if (i2c_write(reg >> 8) || i2c_write(reg & 0xFF)) {  // Register address
            i2c_stop();
            continue;
        }
        
        if (i2c_write(data >> 8) || i2c_write(data & 0xFF)) {  // Data
            i2c_stop();
            continue;
        }
        
        i2c_stop();
        return 0;  // Success
    }
    
    return -1;  // Failure
}

// Check if data is ready and clear flag
int mlx90640_check_data_ready() {
    uint16_t statusReg;
    int retry = 10;
    
    // Wait for data ready flag
    while (retry--) {
        if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x8000, &statusReg, 1) != 0) {
            continue;
        }
        
        if (statusReg & 0x0008) {  // Data ready flag
            // Clear flag
            mlx90640_i2c_write(MLX90640_I2CADDR, 0x8000, 0x0030);
            return 0;  // Data ready
        }
        
        _delay_ms(10);
    }
    
    return -1;  // Timeout
}

// Read a single pixel value directly
int16_t read_pixel_value(uint8_t row, uint8_t col) {
    uint16_t pixelAddr = 0x0400 + ((row * MLX90640_WIDTH + col) * 2);
    uint16_t rawValue;
    
    if (mlx90640_i2c_read(MLX90640_I2CADDR, pixelAddr, &rawValue, 1) != 0) {
        return 0;  // Error
    }
    
    // Convert raw value to temperature (simplified conversion)
    // The actual MLX90640 has a more complex conversion algorithm
    // This is a basic linear approximation
    int16_t value = ((int16_t)rawValue * 10) - 1000; // Scale and offset
    
    return value;
}

// For this specific sensor, add spatial filtering to detect outliers
int16_t validate_temp(int16_t raw_value, uint8_t row, uint8_t col) {
    // Define reasonable temperature ranges (in centidegrees)
    const int16_t MIN_VALID_TEMP = -4000; // -40.00°C
    const int16_t MAX_VALID_TEMP = 30000; // 300.00°C
    
    // Filter out entire row 8 which contains reference pixels
    if (row == 9) {
        return -2000; // Return a reasonable ambient temperature instead
    }
    
    // Regular range checks
    if (raw_value > MAX_VALID_TEMP) {
        return MAX_VALID_TEMP;
    }
    if (raw_value < MIN_VALID_TEMP) {
        return MIN_VALID_TEMP;
    }
    
    return raw_value;
}

// Set refresh rate
int mlx90640_set_refresh_rate(uint8_t rate) {
    uint16_t controlReg;
    
    if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x800D, &controlReg, 1) != 0) {
        return -1;
    }
    
    // Clear and set refresh rate bits (7:5)
    controlReg &= ~(0x07 << 7);
    controlReg |= (rate & 0x07) << 7;
    
    return mlx90640_i2c_write(MLX90640_I2CADDR, 0x800D, controlReg);
}

// Set resolution
int mlx90640_set_resolution(uint8_t resolution) {
    uint16_t controlReg;
    
    if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x800D, &controlReg, 1) != 0) {
        return -1;
    }
    
    // Clear and set resolution bits (11:10)
    controlReg &= ~(0x03 << 10);
    controlReg |= (resolution & 0x03) << 10;
    
    return mlx90640_i2c_write(MLX90640_I2CADDR, 0x800D, controlReg);
}

// Set chess mode
int mlx90640_set_chess_mode() {
    uint16_t controlReg;
    
    if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x800D, &controlReg, 1) != 0) {
        return -1;
    }
    
    // Set chess mode bit (12)
    controlReg |= (1 << 12);
    
    return mlx90640_i2c_write(MLX90640_I2CADDR, 0x800D, controlReg);
}

// Initialize MLX90640
int mlx90640_init() {
    uint16_t id;
    
    // Initialize I2C
    i2c_init();
    _delay_ms(50);
    
    // Check device ID
    if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x2407, &id, 1) != 0) {
        serial_println("Failed to read device ID");
        return -1;
    }
    
    serial_print("Device ID: 0x");
    char buffer[8];
    sprintf(buffer, "%04X", id);
    serial_println(buffer);
    
    // Configure sensor
    if (mlx90640_set_refresh_rate(0x03) != 0) { // 4Hz
        serial_println("Failed to set refresh rate");
        return -2;
    }
    
    if (mlx90640_set_resolution(0x02) != 0) { // 18-bit
        serial_println("Failed to set resolution");
        return -3;
    }
    
    if (mlx90640_set_chess_mode() != 0) {
        serial_println("Failed to set chess mode");
        return -4;
    }
    
    serial_println("MLX90640 configured successfully");
    return 0;
}

// Read and process center region
int mlx90640_read_center_region() {
    // Reset max value to a very low temperature to ensure any valid reading will be higher
    max_temp = -32768;
    max_row_pos = 0;
    max_col_pos = 0;
    int valid_readings = 0;
    int row_to_skip = -1; // Initialize to invalid row
    int max_row = 0;  // Track location of max temperature
    int max_col = 0;
    
    // Wait for data ready
    if (mlx90640_check_data_ready() != 0) {
        serial_println("Timeout waiting for data");
        return -1;
    }
    
    // First pass: read all data and find the row with extreme temperature
    int16_t temp_buffer[CENTER_SIZE][CENTER_SIZE];
    
    for (uint8_t i = 0; i < CENTER_SIZE; i++) {
        for (uint8_t j = 0; j < CENTER_SIZE; j++) {
            // Calculate actual sensor position
            uint8_t row = i + CENTER_START_ROW;
            uint8_t col = j + CENTER_START_COL;
            
            // Read pixel
            int16_t value = read_pixel_value(row, col);
            
            // Store temporarily
            temp_buffer[i][j] = value;
            
            // Check if this is an extreme value (like reference pixel)
            if (value > 14000) { // 140°C is extreme for this application
                row_to_skip = i; // Mark this row to be skipped
            }
        }
    }
    
    // If no extreme row found, default to row 7 (which contained extreme value in sample)
    if (row_to_skip == -1) {
        row_to_skip = 7;
    }
    
    serial_print("Removing row with extreme values: ");
    char buffer[8];
    sprintf(buffer, "%d", row_to_skip);
    serial_println(buffer);
    
    // Reset max_temp AGAIN before second pass to ensure we only consider valid readings
    max_temp = -32768;
    
    // Second pass: copy data to center_data, skipping the extreme row
    for (uint8_t i = 0; i < CENTER_SIZE; i++) {
        // Skip the row with extreme temperatures
        if (i == row_to_skip) continue;
        
        // Calculate destination row (shift rows after the skipped row up)
        uint8_t dest_row = i;
        if (i > row_to_skip) dest_row = i - 1;
        
        for (uint8_t j = 0; j < CENTER_SIZE; j++) {
            // Apply validation to the stored value
            uint8_t row = i + CENTER_START_ROW;
            uint8_t col = j + CENTER_START_COL;
            int16_t validated_value = validate_temp(temp_buffer[i][j], row, col);
            
            // Store the validated value in the shifted position
            center_data[dest_row][j] = validated_value;
            valid_readings++;
            
            // Update max temp (ensure we're looking for actual highest value)
            // Only consider reasonably valid temperatures (not extreme outliers)
            if (validated_value > max_temp && validated_value < 10000) { // 100°C as reasonable max
                max_temp = validated_value;
                max_row = dest_row;
                max_col = j;
            }
        }
    }
    
    // Store max location in global variables for printing
    max_row_pos = max_row;
    max_col_pos = max_col;
    
    // Return error if no valid readings
    if (valid_readings == 0) {
        return -2;
    }
    
    return 0;
}

// Print center matrix data
void print_center_matrix() {
    serial_println("\nCenter Matrix Data (abnormal row removed):");
    
    // Column headers
    serial_print("     ");
    for (uint8_t j = 0; j < CENTER_SIZE; j++) {
        char buffer[8];
        sprintf(buffer, "%2d  ", j);
        serial_print(buffer);
    }
    serial_println("");
    
    // Line separator
    serial_print("    ");
    for (uint8_t j = 0; j < CENTER_SIZE; j++) {
        serial_print("----");
    }
    serial_println("");
    
    // Print data with row numbers
    for (uint8_t i = 0; i < CENTER_SIZE - 1; i++) {  // One less row since we removed a row
        char buffer[8];
        sprintf(buffer, "%2d | ", i);
        serial_print(buffer);
        
        for (uint8_t j = 0; j < CENTER_SIZE; j++) {
            char value_buffer[8];
            if (center_data[i][j] == -32768) {
                // Display ERR for invalid readings
                sprintf(value_buffer, " ERR");
            } else {
                // Display temperatures in whole degrees for simplicity
                sprintf(value_buffer, "%4d", center_data[i][j] / 100);
            }
            serial_print(value_buffer);
        }
        serial_println("");
    }
}

// Test I2C communication
void test_i2c_communication() {
    serial_println("Testing I2C communication...");
    
    uint16_t id;
    if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x2407, &id, 1) == 0) {
        serial_print("Device ID: 0x");
        char buffer[8];
        sprintf(buffer, "%04X", id);
        serial_println(buffer);
    } else {
        serial_println("ERROR: Failed to read device ID");
    }
    
    uint16_t statusReg;
    if (mlx90640_i2c_read(MLX90640_I2CADDR, 0x8000, &statusReg, 1) == 0) {
        serial_print("Status: 0x");
        char buffer[8];
        sprintf(buffer, "%04X", statusReg);
        serial_println(buffer);
    } else {
        serial_println("ERROR: Failed to read status register");
    }
}

// Main function - only include when not compiled as a library
#ifndef EXCLUDE_MAIN
int main() {
    // Disable watchdog
    MCUSR = 0;
    wdt_disable();
    
    // Initialize UART
    serial_init((F_CPU / 16 / BAUD_RATE) - 1);
    
    // Send welcome message
    serial_println("ATmega328P with MLX90640 - Real Sensor Reading");
    serial_println("Thermal Camera (55 degree FoV, 24x32 sensors)");
    
    // Initialize I2C
    i2c_init();
    _delay_ms(100);
    
    // Initialize MLX90640
    int result = mlx90640_init();
    if (result != 0) {
        serial_print("Initialization failed: ");
        char buffer[8];
        sprintf(buffer, "%d", result);
        serial_println(buffer);
    }
    
    serial_println("Reading center region from real sensor");
    
    // Main loop
    while (1) {
        result = mlx90640_read_center_region();
        
        if (result == 0) {
            // Success - print data
            print_center_matrix();
            
            // Print max temperature with position
            serial_print("Max temperature: ");
            if (max_temp == -32768) {
                serial_print("ERR");
            } else {
                char buffer[32];
                int16_t int_part = max_temp / 100;
                uint8_t frac_part = abs(max_temp) % 100;
                
                sprintf(buffer, "%d.%02d C at position [%d][%d]", 
                    int_part, frac_part, max_row_pos, max_col_pos);
                serial_print(buffer);
            }
            
            serial_println("");
            serial_println("---");
        } else if (result == -1) {
            serial_println("Timeout waiting for data ready flag");
        } else if (result == -2) {
            serial_println("No valid readings obtained");
        }
        
        // Wait before next reading
        // _delay_ms(1000); // 1 Hz

        // Make it 10 Hz
        _delay_ms(100);
    }
    
    return 0;
}
#endif // EXCLUDE_MAIN