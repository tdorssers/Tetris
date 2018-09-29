/*
 * Bit-bang I2C routines for SSD1306 controller driver
 *
 * Created: 26-9-2018 08:52:01
 *  Author: Tim Dorssers
 */ 


#ifndef SSD1306_I2C_H_
#define SSD1306_I2C_H_

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

#define DDR_REG  DDRB
#define PORT_REG PORTB
#define SDA      PB0
#define SCL      PB2

#define I2C_WRITE 0

// I2C HIGH = PORT as INPUT(0) and PULL-UP ENABLE (1)
#define I2C_HIGH(DREG, PREG, BIT) { DREG &= ~(1 << BIT); PREG |= (1 << BIT); }

// I2C LOW  = PORT as OUTPUT(1) and OUTPUT LOW (0)
#define I2C_LOW(DREG, PREG, BIT)  { DREG |= (1 << BIT); PREG &= ~(1 << BIT); }

#define I2C_START_STOP_DELAY 0.600
#define I2C_RISE_TIME        0.050
#define I2C_FALL_TIME        0.050
#define I2C_DATA_HOLD_TIME   0.300
#define I2C_IDLE_TIME        1.300
#define I2C_CLOCK            2.500
#define I2C_HALF_CLOCK       ((I2C_CLOCK - I2C_FALL_TIME - I2C_RISE_TIME - I2C_FALL_TIME) / 2)

extern void i2c_stop(void);
extern void i2c_start(uint8_t addr);
extern void i2c_write(uint8_t data);

#endif /* SSD1306_I2C_H_ */