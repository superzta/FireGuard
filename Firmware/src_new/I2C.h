#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// Center 16x16 region
#define CENTER_SIZE 16

// External variables
extern int16_t center_data[CENTER_SIZE][CENTER_SIZE];
extern int16_t max_temp;
extern uint8_t max_row_pos;
extern uint8_t max_col_pos;

// Serial communication functions
void serial_init(unsigned short ubrr);
void serial_print(const char *str);
void serial_println(const char *str);
void print_temp(int16_t value);

// I2C functions
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read(uint8_t ack);

// MLX90640 sensor functions
int mlx90640_init(void);
int mlx90640_read_center_region(void);
void print_center_matrix(void);

#endif /* I2C_H */ 