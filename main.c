/*
 * ATTiny45 Tetris
 *
 * Created: 8-9-2018 15:51:13
 * Author : Tim Dorssers
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "i2cmaster.h"
#include "ssd1306.h"

char buffer[4];
uint8_t buttonLeft, buttonRight, buttonDown, buttonUp, buttonA, buttonB;
#define WELL_MAX 30
uint16_t well[WELL_MAX];
int8_t pieceX, pieceY;
#define NO_PIECE 255
uint8_t piece, rotate, nextPiece, holdPiece = NO_PIECE;
uint16_t random_number = 1;
#define CD_DROP   0
#define CD_ROTATE 1
#define CD_LEFT   2
#define CD_RIGHT  3
#define CD_LOCK   4
const uint8_t PROGMEM mask[] = {0xDB, 0xB6, 0x6D, 0xDB};
const uint16_t PROGMEM pieces[] = {
	0xF, 0x2222, 0xF, 0x2222,  // I
	0x17, 0x223, 0x74, 0x622,  // J
	0x47, 0x322, 0x71, 0x226,  // L
	0x66, 0x66, 0x66, 0x66,    // O
	0x63, 0x264, 0x63, 0x264,  // S
	0x27, 0x232, 0x72, 0x262,  // T
	0x36, 0x462, 0x36, 0x462   // Z
};

// Draw one line of the specified piece at position y in row
void drawPiece(uint8_t x, uint8_t y, uint8_t p, uint32_t *row) {
	uint8_t i, xx, yy;
	
	xx = x * 4;
	yy = (y * 3) + 1;
	for (i = xx; i < xx + 4; i++) {
		if (pgm_read_word(&pieces[p]) & 1 << i) {
			*row |= 7UL << yy;
		}
		yy += 3;
	}
}

// Render screen for each ssd1306 page
void drawScreen(void) {
	uint8_t x, y, p, i;
	uint32_t row;
	
	for (y = 0; y < 4; y++) {
		ssd1306_set_cursor(0, y);
		ssd1306_send_data_start();
		i2c_write(0xFF); // Bottom line of well
		for (x = 0; x < WELL_MAX + 4; x++) {
			if (x < WELL_MAX) {
				row = 1 << 0 | 1UL << 31; // Side lines of well
				for (i = 0; i < 10; i++) {
					if (well[x] & 1 << i) {
						row |= 7UL << ((i * 3) + 1); // 3 pixels for each block
					}
				}
				// Draw line of the current piece in row
				if (x >= pieceX && x < pieceX + 4) {
					drawPiece(x - pieceX, pieceY, piece * 4 + rotate, &row);
				}
			} else if (x == WELL_MAX || x == WELL_MAX + 3) {
				i2c_write(0xFF); // Top and bottom lines of rectangles
				continue;
			} else {
				// Draw sides of rectangles with hold and next pieces
				row = 1 << 0 | 1UL << 15 | 1UL << 31;
				drawPiece(x - (WELL_MAX + 1), 6, nextPiece * 4, &row);
				if (holdPiece != NO_PIECE)
					drawPiece(x - (WELL_MAX + 1), 0, holdPiece * 4, &row);
			}
			// Select page, draw 3 rows and apply mask to middle row
			p = ((uint8_t *)&row)[y];
			i2c_write(p);
			i2c_write(p & pgm_read_byte(&mask[y]));
			i2c_write(p);
		}
		i2c_stop();
	}
}

// Collision detect and piece locking procedure
uint8_t collisionDetect(uint8_t mode) {
	uint8_t x, i;
	int8_t y;
	
	for (i = 0; i < 16; i++) {
		x = pieceX + (i / 4);
		y = pieceY + i % 4;
		switch (mode) {
			case CD_DROP: x--; break;
			case CD_LEFT: y--; break;
			case CD_RIGHT: y++;
		}
		if (pgm_read_word(&pieces[piece * 4 + rotate]) & 1 << i) {
			if (mode == CD_LOCK)
				well[x] |= 1 << y;
			else {
				if (well[x] & 1 << y)
					return true;
				if ((mode == CD_ROTATE || mode == CD_LEFT) && y < 0)
					return true;
				if ((mode == CD_ROTATE || mode == CD_RIGHT) && y > 9)
					return true;
			}
		}
	}
	return false;
}

// Returns number of cleared lines
uint8_t clearLine(void) {
	uint8_t x, xx, s = 0;
	
	for (x = 0; x < WELL_MAX; x++) {
		if (well[x] >= 0x3FF) {
			for (xx = x; xx < WELL_MAX - 1; xx++) {
				well[xx] = well[xx + 1];
			}
			well[WELL_MAX - 1] = 0;
			s++;
		}
	}
	return s;
}

// Galois Linear Feedback Shift Register
uint16_t lfsr16_next(uint16_t n) {
	return (n >> 1) ^ (-(n & 0x1) & 0xB400);
}

// Pseudo Random Number Generator
uint16_t prng(void) {
	return (random_number = lfsr16_next(random_number));
}

// Put next piece on playing field
void newPiece(void) {
	pieceX = WELL_MAX - 2;
	pieceY = 3;
	piece = nextPiece;
	rotate = 0;
	// NES Tetris-like randomizer
	nextPiece = prng() % 8;
	if (nextPiece == 7 || nextPiece == piece) {
		nextPiece = prng() % 7;
	}
}

// Initialize button matrix
inline void matrix_init(void) {
	PORTB |= _BV(1) | _BV(3) | _BV(4); // enable pull up
}

// Scan button matrix
void scanMatrix(void) {
	buttonA = false;
	buttonB = false;
	buttonDown = false;
	buttonLeft = false;
	buttonRight = false;
	buttonUp = false;
	DDRB |= _BV(1);   // PB1 as output
	PORTB &= ~_BV(1); // PB1 low
	_delay_us(150);
	if (bit_is_clear(PINB, 4)) {
		if (bit_is_clear(PINB, 3))
			buttonB = true;
		else
			buttonRight = true;
	}	
	DDRB &= ~_BV(1); // PB1 as input
	PORTB |= _BV(1); // PB1 pull up
	DDRB |= _BV(3);  // PB3 as output
	PORTB &= ~_BV(3);// PB3 low
	_delay_us(150);
	if (bit_is_clear(PINB, 4)) {
		if (bit_is_clear(PINB, 1))
			buttonLeft = true;
		else
			buttonA = true;
	}
	DDRB &= ~_BV(3); // PB3 as input
	PORTB |= _BV(3); // PB3 pull up
	DDRB |= _BV(4);  // PB4 as output
	PORTB &= ~_BV(4);// PB4 low
	_delay_us(150);
	if (bit_is_clear(PINB, 1))
		buttonDown = true;
	if (bit_is_clear(PINB, 3))
		buttonUp = true;
	DDRB &= ~_BV(4); // PB4 as input
	PORTB |= _BV(4); // PB4 pull up
}

// Setup game screen
void setupScreen(void) {
	ssd1306_clear();
	ssd1306_on();
	ssd1306_string_font6x8_cw_p(120, PSTR("Scor"));
	ssd1306_string_font6x8_cw_p(104, PSTR("Lv"));
	memset(well, 0, sizeof(well));
}

// End of game screen
void gameOver(void) {
	uint8_t cnt = 0xFF;
	
	ssd1306_string_font6x8_cw_p(64, PSTR("Game"));
	ssd1306_string_font6x8_cw_p(56, PSTR("Over"));
	while (!buttonLeft && !buttonRight && !buttonUp && !buttonDown && !buttonA && !buttonB) {
		scanMatrix();
		_delay_ms(40);
		if (cnt-- == 0)
			ssd1306_off();
	}
	setupScreen();
}

// Main loop
int main(void) {
	uint8_t dropPiece = false, mayHold = true, speed = 10, level = 0, lines = 0, temp;
	uint8_t holdButtonUp = false, holdButtonB = false, holdButtonLeft = 0, holdButtonRight = 0;
	uint16_t score = 0;
	
	matrix_init();
	i2c_init();
	ssd1306_init();
	setupScreen();
	newPiece();
	while (1)
    {
		// Handle left button
		if (buttonLeft) {
			if (!collisionDetect(CD_LEFT) && (holdButtonLeft == 0 || holdButtonLeft > 9))
				pieceY--;
			if (holdButtonLeft < 10)
				holdButtonLeft++;
			prng();
		}
		if (!buttonLeft && holdButtonLeft)
			holdButtonLeft = 0;
		// Handle right button
		if (buttonRight) {
			if (!collisionDetect(CD_RIGHT) && (holdButtonRight == 0 || holdButtonRight > 9))
				pieceY++;
			if (holdButtonRight < 10)
				holdButtonRight++;
			prng();
		}
		if (!buttonRight && holdButtonRight)
			holdButtonRight = 0;
		// Handle up button (rotate)
		if (buttonUp && !holdButtonUp) {
			holdButtonUp = true;
			temp = rotate;
			rotate = (rotate + 1) & 0x3;
			// Restore previous rotation if there is no space to rotate
			if (collisionDetect(CD_ROTATE))
				rotate = temp;
			prng();
		}
		if (!buttonUp && holdButtonUp)
			holdButtonUp = false;
		// Handle B button (hard drop)
		if (buttonB && !holdButtonB) {
			holdButtonB = true;
			dropPiece = true;
			prng();
		}
		if (!buttonB && holdButtonB)
			holdButtonB = false;
		// Handle A button (hold)
		if (buttonA && mayHold) {
			mayHold = false;
			if (holdPiece == NO_PIECE) {
				holdPiece = piece;
				newPiece();
			} else {
				temp = piece;
				piece = holdPiece;
				holdPiece = temp;
				pieceX = WELL_MAX - 2;
				pieceY = 3;
				rotate = 0;
			}
			prng();
		}
		// Handle down button (soft drop)
		if (buttonDown) { 
			speed = 0;
			prng();
		}
		// Check if piece can't drop further
		if (collisionDetect(CD_DROP)) {
			// Lock piece
			collisionDetect(CD_LOCK);
			// Check if well is full
			if (pieceX >= WELL_MAX - 3) {
				gameOver();
				nextPiece = 0;
				holdPiece = NO_PIECE;
				score = 0;
				level = 0;
			}
			mayHold = true;
			dropPiece = false;
			newPiece();
		}
		// Drop piece with delay except when hard dropping
		if (speed-- == 0 || dropPiece) {
			speed = 10 - level;
			if (pieceX > 0)
				pieceX--;
			else {
				// Lock piece at bottom
				collisionDetect(CD_LOCK);
				mayHold = true;
				dropPiece = false;
				newPiece();
			}
		}
		// Clear full lines and scoring system
		if ((temp = clearLine())) {
			lines += temp;
			switch (temp) {
				case 2: temp = 3; break;
				case 3: temp = 5; break;
				case 4: temp = 8;
			}
			score += (temp * (level + 1));
			level = lines / 10;
			if (level > 9)
				level = 9; 
		}
		// Draw screen and delay except while hard dropping
		if (!dropPiece) {
			drawScreen();
			// Display level and score
			ssd1306_set_cursor(104, 3);
			ssd1306_char_font6x8_cw('0' + level);
			utoa(score, buffer, 10);
			ssd1306_string_font6x8_cw(112, buffer);
			// Scan button matrix
			scanMatrix();
			_delay_ms(40);
		}
    }
}

