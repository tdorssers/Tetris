/*
 * Bit-bang I2C routines for SSD1306 controller driver
 *
 * Created: 26-9-2018 08:51:43
 *  Author: Tim Dorssers
 */ 

#include "ssd1306_i2c.h"

void i2c_write(uint8_t data) {
	uint8_t i;
	
	for (i = 8; i > 0; i--) {
		if (data & 0x80)
			I2C_HIGH(DDR_REG, PORT_REG, SDA)
		else
			I2C_LOW(DDR_REG, PORT_REG, SDA);
		data <<= 1;
		_delay_us(I2C_FALL_TIME);
		I2C_HIGH(DDR_REG, PORT_REG, SCL);
		_delay_us(I2C_HALF_CLOCK);
		I2C_LOW(DDR_REG, PORT_REG, SCL);
		_delay_us(I2C_HALF_CLOCK);
	}
	// ACK
	I2C_HIGH(DDR_REG, PORT_REG, SDA);
	_delay_us(I2C_FALL_TIME);
	I2C_HIGH(DDR_REG, PORT_REG, SCL);
	_delay_us(I2C_HALF_CLOCK);
	I2C_LOW(DDR_REG, PORT_REG, SCL);
	_delay_us(I2C_HALF_CLOCK);
}

void i2c_start(uint8_t addr) {
	I2C_LOW(DDR_REG, PORT_REG, SDA);    // Set to LOW
	_delay_us(I2C_START_STOP_DELAY);
	I2C_LOW(DDR_REG, PORT_REG, SCL);    // Set to LOW
	_delay_us(I2C_HALF_CLOCK);
	i2c_write(addr);
}

void i2c_stop(void) {
	I2C_LOW(DDR_REG, PORT_REG, SDA);	// Set to LOW
	_delay_us(I2C_FALL_TIME);
	I2C_HIGH(DDR_REG, PORT_REG, SCL);	// Set to HIGH
	_delay_us(I2C_START_STOP_DELAY);
	I2C_HIGH(DDR_REG, PORT_REG, SDA);	// Set to HIGH
	_delay_us(I2C_IDLE_TIME);
}
