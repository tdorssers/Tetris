/*
 * Title   : Tetris
 * Hardware: ATtiny45 @ 16 MHz, SSD1306 I2C 128x32 OLED, 6 tactile buttons
 * Created : 8-9-2018 15:51:13
 * Author  : Tim Dorssers
 *
 * Portrait screen orientation is used, for efficient use of the screen area.
 * The SSD1306 controller, capable of driving an 128x64 OLED screen, has 1K
 * SRAM. When driving an 128x32 OLED, only 512 bytes are needed. The ATtiny45
 * has just 256 bytes of SRAM, which is not enough to hold a frame buffer.
 * Therefore the screen is rendered in rows of 32 bits and each row is sent in
 * four one byte pages over the I2C bus to the display controller. Up to 47
 * frames per second are rendered, depending on the amount of blocks in the
 * playing field. Pushing the up and down button simultaneously displays the
 * FPS rate. The game uses a 10x30 playing field and implements hard and soft
 * dropping of the pieces, as well as delayed auto shift (DAS), entry delay
 * (ARE), piece preview, hold piece and the Super Rotation System.
 * In game power draw is <20 mA and standby power draw is <1 mA.
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "i2cmaster.h"
#include "ssd1306.h"

// Buffers for font drawing
char buffer[6];
uint32_t bitmap[8];
// Button state variables
bool buttonLeft, buttonRight, buttonDown, buttonUp, buttonA, buttonB;
// Play field is 10 bits wide and 30 rows tall
#define WELL_MAX 30
uint16_t well[WELL_MAX];
// Piece variables
#define NO_PIECE 255
int8_t pieceX, pieceY;
uint8_t piece, rotate, nextPiece, holdPiece = NO_PIECE;
// Collision detect mode enum
typedef enum {CD_DROP, CD_ROTATE, CD_LEFT, CD_RIGHT, CD_LOCK} mode_t;
// Random number seed variable
uint16_t random_number = 1;
// Delay constants
#define DROP_DELAY  20
#define LOCK_DELAY  20
#define ENTRY_DELAY 20
#define SHIFT_DELAY 10
#define SOFT_DELAY  2
// Millisecond counter
volatile uint16_t timer0_millis;
bool showFps = false;
// Mask for play field
const uint8_t PROGMEM mask[] = {0xDB, 0xB6, 0x6D, 0xDB};
// Each piece is 4x4 bits
const uint16_t PROGMEM pieces[] = {
	0xF00, 0x4444, 0xF0, 0x2222, // I
	0x170, 0x622, 0x74, 0x223,   // J
	0x470, 0x226, 0x71, 0x322,   // L
	0x660, 0x660, 0x660, 0x660,  // O
	0x630, 0x264, 0x63, 0x132,   // S
	0x270, 0x262, 0x72, 0x232,   // T
	0x360, 0x462, 0x36, 0x231    // Z
};
const uint8_t PROGMEM bmpScore[] = {
	0x0, 0x8F, 0x50, 0x50, 0x4E, 0x81, 0x1, 0x1E, 
	0x0, 0xE3, 0x14, 0x10, 0x10, 0xE3, 0x0, 0x0, 
	0x0, 0x4, 0x5, 0x5, 0x4D, 0x34, 0x0, 0x0, 
	0x0, 0xE, 0x1, 0x1F, 0x11, 0xE, 0x0, 0x0
};
const uint8_t PROGMEM bmpLevel[] = {
	0x0, 0x9F, 0x41, 0xC1, 0x41, 0x81, 0x1, 0x1, 
	0x0, 0x43, 0xA0, 0x17, 0x14, 0x13, 0x0, 0x0, 
	0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x0, 0x0
};
const uint8_t PROGMEM bmpGameOver[] = {
	0x1, 0xE1, 0x11, 0x11, 0x11, 0x11, 0x11, 0xE1, 
	0x1, 0xE1, 0x11, 0x11, 0xD1, 0x11, 0x11, 0xE1, 
	0x0, 0x10, 0x29, 0x45, 0x45, 0x45, 0x1, 0x0, 
	0x0, 0x79, 0x45, 0x79, 0x41, 0x38, 0x1, 0x0, 
	0x0, 0x4E, 0x41, 0x5F, 0xD1, 0x4E, 0x0, 0x0, 
	0x0, 0x91, 0x51, 0xD5, 0x55, 0x8B, 0x0, 0x0, 
	0x80, 0x80, 0x80, 0x80, 0x84, 0x83, 0x80, 0x80,
	0x80, 0x83, 0x80, 0x87, 0x84, 0x83, 0x80, 0x80
};
const uint8_t PROGMEM bmpFPS[] = {
	0x0, 0x41, 0x41, 0x41, 0xCF, 0x41, 0x41, 0xDF,
	0x0, 0xF0, 0x0, 0x0, 0xE3, 0x14, 0x14, 0xE3,
	0x0, 0x0, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
const uint8_t PROGMEM font6x8_90[] = {
	0x0, 0xE, 0x11, 0x13, 0x15, 0x19, 0x11, 0xE, // 0
	0x0, 0xE, 0x4, 0x4, 0x4, 0x4, 0x6, 0x4,      // 1
	0x0, 0x1F, 0x2, 0x4, 0x8, 0x10, 0x11, 0xE,   // 2
	0x0, 0xE, 0x11, 0x10, 0x8, 0x4, 0x8, 0x1F,   // 3
	0x0, 0x8, 0x8, 0x1F, 0x9, 0xA, 0xC, 0x8,     // 4
	0x0, 0xE, 0x11, 0x10, 0x10, 0xF, 0x1, 0x1F,  // 5
	0x0, 0xE, 0x11, 0x11, 0xF, 0x1, 0x2, 0xC,    // 6
	0x0, 0x2, 0x2, 0x2, 0x4, 0x8, 0x10, 0x1F,    // 7
	0x0, 0xE, 0x11, 0x11, 0xE, 0x11, 0x11, 0xE,  // 8
	0x0, 0x6, 0x8, 0x10, 0x1E, 0x11, 0x11, 0xE   // 9
};

// Prototypes
uint8_t clearLine(void);
bool collisionDetect(mode_t mode);
void drawHeader(void);
void drawPiece(uint8_t x, uint8_t y, uint8_t p, uint32_t *row);
void drawScreen(void);
void drawValue(uint8_t x, uint8_t y, uint16_t v);
void eraseScore(void);
void gameOver(void);
uint16_t lfsr16_next(uint16_t n);
inline void matrix_init(void);
void newPiece(void);
uint16_t prng(void);
void scanMatrix(void);
void setupScreen(void);
void swapPiece(void);
inline void timer0_init();
void waitRelease(void);

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
		for (x = 0; x < WELL_MAX + 5; x++) {
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
			} else if (x == WELL_MAX || x == WELL_MAX + 4) {
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

// Draws maximal 5 digits of 6x8 pixels at position x starting on page y
void drawValue(uint8_t x, uint8_t y, uint16_t v) {
	uint8_t i = 0, j, c, offset, shift;
	
	memset(bitmap, 0, sizeof(bitmap));
	utoa(v, buffer, 10);
	shift = y * 8;
	while ((c = buffer[i])) {
		offset = (c - 48) * 8;
		for (j = 0; j < 8; j++)
			bitmap[j] |= (uint32_t)pgm_read_byte(&font6x8_90[offset++]) << shift;
		shift += 6;
		i++;
	}
	for (i = y; i < 4; i++) {
		ssd1306_set_cursor(x, i);
		ssd1306_send_data_start();
		for (j = 0; j < 8; j++) {
			i2c_write(((uint8_t *)&bitmap[j])[i]);
		}
		i2c_stop();
	}
}

// Clear score area
void eraseScore(void) {
	uint8_t i;
	
	for (i = 0; i < 4; i++) {
		ssd1306_set_cursor(112, i);
		ssd1306_fill_length(0, 8);
	}
}

// Draw FPS or score bitmap
void drawHeader(void) {
	ssd1306_bitmap_p(120, 0, 128, 4, showFps ? bmpFPS : bmpScore);
}

// Setup game screen
void setupScreen(void) {
	ssd1306_clear();
	ssd1306_on();
	//ssd1306_bitmap_p(120, 0, 128, 4, bmpScore);
	drawHeader();
	ssd1306_bitmap_p(104, 0, 112, 3, bmpLevel);
	memset(well, 0, sizeof(well));
}

// Dummy ISR
EMPTY_INTERRUPT(WDT_vect);

// End of game screen and sleep mode
void gameOver(void) {
	uint8_t cnt = 80;
	
	ssd1306_bitmap_p(56, 0, 72, 4, bmpGameOver);
	cli();
	WDTCR = _BV(WDCE) | _BV(WDE);				// Watchdog change enable
	WDTCR = _BV(WDIE) | _BV(WDP1) | _BV(WDP0);	// Watchdog timeout interrupt enable, period 0.125 s
	sei();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	do {
		scanMatrix();
		sleep_mode(); // Put the device into sleep mode
		if (--cnt == 0)
			ssd1306_off();
	} while (!buttonLeft && !buttonRight && !buttonUp && !buttonDown && !buttonA && !buttonB);
	MCUSR = 0x00;					// Clear watchdog reset flag
	WDTCR = _BV(WDCE) | _BV(WDE);	// Watchdog change enable
	WDTCR = 0x00;					// Disable watchdog
	waitRelease();
	setupScreen();
}

// Collision detect and piece locking procedure
bool collisionDetect(mode_t mode) {
	uint8_t i;
	int8_t x, y;
	
	for (i = 0; i < 16; i++) {
		x = pieceX + (i / 4);
		y = pieceY + i % 4;
		switch (mode) {
			case CD_DROP: x--; break;
			case CD_LEFT: y--; break;
			case CD_RIGHT: y++;
			default: ;
		}
		if (pgm_read_word(&pieces[piece * 4 + rotate]) & 1 << i) {
			if (mode == CD_LOCK)
				well[x] |= 1 << y;
			else {
				if (x < 0 || well[x] & 1 << y)
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

// Clear rows of blocks that span entire playing field. Returns number of cleared lines
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
	pieceX = WELL_MAX - 3;
	pieceY = 3;
	piece = nextPiece;
	rotate = 0;
	// NES-like randomizer
	nextPiece = prng() % 8;
	if (nextPiece == 7 || nextPiece == piece) {
		nextPiece = prng() % 7;
	}
}

// Swap falling piece with hold piece
void swapPiece(void) {
	uint8_t temp;
	
	temp = piece;
	piece = holdPiece;
	holdPiece = temp;
	pieceX = WELL_MAX - 3;
	pieceY = 3;
	rotate = 0;
}

// Initialize button matrix
inline void matrix_init(void) {
	PORTB |= _BV(1) | _BV(3) | _BV(4); // enable pull up
}

// Scan button matrix
void scanMatrix(void) {
	buttonA = buttonB = buttonDown = buttonLeft = buttonRight = buttonUp = false;
	DDRB |= _BV(1);   // PB1 as output
	PORTB &= ~_BV(1); // PB1 low
	_delay_us(75);
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
	_delay_us(75);
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
	_delay_us(75);
	if (bit_is_clear(PINB, 1))
		buttonDown = true;
	if (bit_is_clear(PINB, 3))
		buttonUp = true;
	DDRB &= ~_BV(4); // PB4 as input
	PORTB |= _BV(4); // PB4 pull up
}

// Waits for all buttons to be released
void waitRelease(void) {
	do {
		scanMatrix();
	} while (buttonLeft || buttonRight || buttonUp || buttonDown || buttonA || buttonB);
}

// Count milliseconds
ISR(TIMER0_COMPA_vect) {
	timer0_millis++;
}

// Get current millis
uint16_t millis(void) {
	uint16_t ms;
	
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ms = timer0_millis;
	}
	return ms;
}

// Initialize 1 millisecond timer
inline void timer0_init() {
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS00) | _BV(CS01);
	OCR0A = (F_CPU / 1000) / 64;
	TIMSK = _BV(OCIE0A);
	sei();
}

// Main loop
int main(void) {
	bool dropPiece = false, mayHold = true, holdButtonUp = false, holdButtonB = false;
	uint8_t holdButtonLeft = 0, holdButtonRight = 0, level = 0, lines = 0, temp;
	uint8_t dropDelay = DROP_DELAY + ENTRY_DELAY, lockDelay = LOCK_DELAY, dropScore = 0;
	uint16_t score = 0, start, fps = 0;
	
	matrix_init();
	i2c_init();
	ssd1306_init();
	timer0_init();
	setupScreen();
#ifdef DOUBLE_BUFFER
	ssd1306_switchFrame();
	setupScreen();
#endif
	newPiece();
	while (1) {
		start = millis();
		// Concurrent pushing of up and down button toggles displaying fps or score
		if (buttonUp && buttonDown) {
			showFps ^= true;
			drawHeader();
			waitRelease();
			eraseScore();
		}
		// Handle left button
		if (buttonLeft) {
			if (!collisionDetect(CD_LEFT) && (holdButtonLeft == 0 || holdButtonLeft >= SHIFT_DELAY))
				pieceY--;
			if (holdButtonLeft < SHIFT_DELAY)
				holdButtonLeft++; // Delay auto repeat
			else
				holdButtonLeft--; // Skip one frame
			prng();
		}
		if (!buttonLeft && holdButtonLeft)
			holdButtonLeft = 0;
		// Handle right button
		if (buttonRight) {
			if (!collisionDetect(CD_RIGHT) && (holdButtonRight == 0 || holdButtonRight >= SHIFT_DELAY))
				pieceY++;
			if (holdButtonRight < SHIFT_DELAY)
				holdButtonRight++; // Delay auto repeat
			else
				holdButtonRight--; // Skip one frame
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
			} else
				swapPiece();
			dropScore = 0;
			prng();
		}
		// Handle down button (soft drop)
		if (buttonDown && dropDelay > SOFT_DELAY) { 
			dropDelay = SOFT_DELAY;
			dropScore++;
			prng();
		}
		// Check if piece can't drop further
		if (collisionDetect(CD_DROP)) {
			// Lock piece when timer expires or immediately when hard or soft dropping
			if (lockDelay-- == 0 || dropPiece || dropDelay == SOFT_DELAY) {
				lockDelay = LOCK_DELAY;
				dropDelay = ENTRY_DELAY + DROP_DELAY - (level * (DROP_DELAY / 10));
				mayHold = true;
				dropPiece = false;
				// Drop scoring
				score += dropScore / 8;
				dropScore = 0;
				// Lock piece
				collisionDetect(CD_LOCK);
				// Spawn new piece and check if well is full
				newPiece();
				if (collisionDetect(CD_ROTATE)) {
					gameOver();
					nextPiece = 0;
					holdPiece = NO_PIECE;
					score = 0;
					level = 0;
					lines = 0;
				}
			}
		}
		// Drop piece when timer expires or immediately when hard dropping
		if (--dropDelay == 0 || dropPiece) {
			dropDelay = DROP_DELAY - (level * (DROP_DELAY / 10));
			if (dropPiece)
				dropScore += 2;
			pieceX--;
		}
		// Clear full lines and scoring system
		if ((temp = clearLine())) {
			lines += temp;
			switch (temp) {
				case 1: temp = 10; break;
				case 2: temp = 30; break;
				case 3: temp = 50; break;
				default: temp = 80;
			}
			score += (temp * (level + 1));
			level = lines / 10;
			if (level > 9)
				level = 9; 
		}
		// Draw screen except when hard dropping
		if (!dropPiece) {
			drawScreen();
			// Display level and score
			drawValue(104, 3, level);
			drawValue(112, 0, showFps ? fps : score);
#ifdef DOUBLE_BUFFER
			ssd1306_switchFrame();
#endif
			// Scan button matrix
			scanMatrix();
			// Calculate fps
			fps = 1000 / (millis() - start);
			// Maintain 40 fps
			while ((millis() - start) < 25);
		}
    }
}

