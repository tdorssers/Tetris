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
 * The highest score is stored in EEPROM as well as the players name. The game
 * will enter sleep mode automatically. The game wakes up by a button push.
 * In game power draw is <20 mA and standby power draw is <1 mA.
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ssd1306_i2c.h"
#include "ssd1306.h"

//#define DOUBLE_BUFFER
//#define DEBUG_FPS

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
uint16_t random_number;
// Delay constants
#define DROP_DELAY  20
#define LOCK_DELAY  20
#define ENTRY_DELAY 20
#define SHIFT_DELAY 10
#define SOFT_DELAY  2
// Millisecond counter
volatile uint16_t timer0_millis;
// Non volatile
uint16_t EEMEM nvHighScore = 0;
uint8_t EEMEM nvName[6] = "";
uint16_t EEMEM nvRandomSeed = 1;
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
const uint8_t PROGMEM font6x8_90[] = {
	0xE, 0x11, 0x13, 0x15, 0x19, 0x11, 0xE,   // 0
	0xE, 0x4, 0x4, 0x4, 0x4, 0x6, 0x4,        // 1
	0x1F, 0x2, 0x4, 0x8, 0x10, 0x11, 0xE,     // 2
	0xE, 0x11, 0x10, 0x8, 0x4, 0x8, 0x1F,     // 3
	0x8, 0x8, 0x1F, 0x9, 0xA, 0xC, 0x8,       // 4
	0xE, 0x11, 0x10, 0x10, 0xF, 0x1, 0x1F,    // 5
	0xE, 0x11, 0x11, 0xF, 0x1, 0x2, 0xC,      // 6
	0x2, 0x2, 0x2, 0x4, 0x8, 0x10, 0x1F,      // 7
	0xE, 0x11, 0x11, 0xE, 0x11, 0x11, 0xE,    // 8
	0x6, 0x8, 0x10, 0x1E, 0x11, 0x11, 0xE,    // 9
	0x11, 0x11, 0x1F, 0x11, 0x11, 0xA, 0x4,   // A
	0xF, 0x11, 0x11, 0xF, 0x11, 0x11, 0xF,    // B
	0xE, 0x11, 0x1, 0x1, 0x1, 0x11, 0xE,      // C
	0x7, 0x9, 0x11, 0x11, 0x11, 0x9, 0x7,     // D
	0x1F, 0x1, 0x1, 0xF, 0x1, 0x1, 0x1F,      // E
	0x1, 0x1, 0x1, 0xF, 0x1, 0x1, 0x1F,       // F
	0x1E, 0x11, 0x11, 0x1D, 0x1, 0x11, 0xE,   // G
	0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, // H
	0xE, 0x4, 0x4, 0x4, 0x4, 0x4, 0xE,        // I
	0x6, 0x9, 0x8, 0x8, 0x8, 0x8, 0x1C,       // J
	0x11, 0x9, 0x5, 0x3, 0x5, 0x9, 0x11,      // K
	0x1F, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,       // L
	0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11, // M
	0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, // N
	0xE, 0x11, 0x11, 0x11, 0x11, 0x11, 0xE,   // O
	0x1, 0x1, 0x1, 0xF, 0x11, 0x11, 0xF,      // P
	0x16, 0x9, 0x15, 0x11, 0x11, 0x11, 0xE,   // Q
	0x11, 0x9, 0x5, 0xF, 0x11, 0x11, 0xF,     // R
	0xF, 0x10, 0x10, 0xE, 0x1, 0x1, 0x1E,     // S
	0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x1F,       // T
	0xE, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // U
	0x4, 0xA, 0x11, 0x11, 0x11, 0x11, 0x11,   // V
	0xA, 0x15, 0x15, 0x15, 0x11, 0x11, 0x11,  // W
	0x11, 0x11, 0xA, 0x4, 0xA, 0x11, 0x11,    // X
	0x4, 0x4, 0x4, 0xA, 0x11, 0x11, 0x11,     // Y
	0x1F, 0x1, 0x2, 0x4, 0x8, 0x10, 0x1F      // Z
};
const char PROGMEM pstrScore[] = "SCORE";

// Prototypes
uint8_t clearLine(void);
bool collisionDetect(mode_t mode);
void drawPiece(uint8_t x, uint8_t y, uint8_t p, uint32_t *row);
void drawScreen(void);
void drawString_p(uint8_t x, uint8_t y, const char *s);
void drawValue(uint8_t x, uint8_t y, uint16_t v);
inline uint16_t lfsr16_next(uint16_t n);
inline void matrix_init(void);
uint16_t millis(void);
void newPiece(void);
uint16_t prng(void);
inline void prng_init(void);
void scanMatrix(void);
void scoreScreen (uint16_t score);
void setupScreen(void);
void sleepMode(void);
inline void swapPiece(void);
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

// Draws maximal 5 digits or caps of 6x8 pixels at position x starting on page y
void drawString_p(uint8_t x, uint8_t y, const char *s) {
	uint8_t i = 0, j, c, shift;
	uint16_t offset;
	
	memset(bitmap, 0, sizeof(bitmap));
	shift = y * 8;
	while ((c = (s) ? pgm_read_byte(s + i) : buffer[i])) {		
		if (c > 32) {
			if (c > 64)
				c -= 7;
			offset = (uint16_t)(c - 48) * 7;
			for (j = 1; j < 8; j++)
				bitmap[j] |= (uint32_t)pgm_read_byte(&font6x8_90[offset++]) << shift;
		}
		shift += 6;
		i++;
	}
	for (i = y; i < 4; i++) {
		ssd1306_set_cursor(x, i);
		ssd1306_send_data_start();
		for (j = 0; j < 8; j++)
			i2c_write(((uint8_t *)&bitmap[j])[i]);
		i2c_stop();
	}
}

void drawValue(uint8_t x, uint8_t y, uint16_t v) {
	utoa(v, buffer, 10);
	drawString_p(x, y, NULL);
}

// Setup game screen
void setupScreen(void) {
	ssd1306_clear();
	ssd1306_on();
#ifdef DEBUG_FPS
	drawString_p(120, 0, PSTR("FPS  "));
#else
	drawString_p(120, 0, pstrScore);
#endif
	drawString_p(104, 0, PSTR("LEV"));
	memset(well, 0, sizeof(well));
}

// End of game screen
void scoreScreen (uint16_t score) {
	bool blink = false;
	uint8_t i = 0, c = 65, delay = 0, cnt = 80;
	uint16_t highScore;
	
	drawString_p(72, 0, PSTR(" GAME"));
	drawString_p(64, 0, PSTR(" OVER"));
	drawString_p(56, 0, PSTR("HIGH "));
	drawString_p(48, 0, pstrScore);
	if (score < (highScore = eeprom_read_word(&nvHighScore))) {
		eeprom_read_block(&buffer, &nvName, sizeof(buffer));
		drawString_p(40, 0, NULL);
		drawValue(32, 0, highScore);
		return;
	}
	drawString_p(40, 0, PSTR(" NAME"));
	memset(buffer, 0, sizeof(buffer));
	do {
		scanMatrix();
		if (buttonLeft && i > 0) {
			waitRelease();
			i--;
		}
		if (buttonRight && i < 4) {
			waitRelease();
			i++;
		}
		if (buttonUp) {
			waitRelease();
			if (c == 32)
				c = 65;
			else {
				if (c < 90)
					c++;
				else
					c = 32;
			}
		}
		if (buttonDown) {
			waitRelease();
			if (c == 32)
				c = 90;
			else {
				if (c > 65)
					c--;
				else
					c = 32;
			}
		}
		buffer[i] = (blink) ? 32 : c;
		drawString_p(32, 0, NULL);
		buffer[i] = c;
		if (--delay == 0) {
			blink ^= true;
			if (--cnt == 0)
				break;
		}
	} while (!buttonA && !buttonB);
	waitRelease();
	drawString_p(32, 0, NULL);
	eeprom_write_block(&buffer, &nvName, sizeof(nvName));
	eeprom_write_word(&nvHighScore, score);
}

// Dummy ISR
EMPTY_INTERRUPT(WDT_vect);

// Sleep mode
void sleepMode(void) {
	uint8_t cnt = 80;
	
	eeprom_write_word(&nvRandomSeed, random_number);
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
inline uint16_t lfsr16_next(uint16_t n) {
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
inline void swapPiece(void) {
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

// Read random seed from EEPROM
inline void prng_init(void) {
	random_number = eeprom_read_word(&nvRandomSeed);
	newPiece();
	newPiece();
}

// Main loop
int main(void) {
	bool dropPiece = false, mayHold = true, holdButtonUp = false, holdButtonB = false;
	uint8_t holdButtonLeft = 0, holdButtonRight = 0, level = 0, lines = 0, temp;
	uint8_t dropDelay = DROP_DELAY + ENTRY_DELAY, lockDelay = LOCK_DELAY, dropScore = 0;
	uint16_t score = 0, start;
#ifdef DEBUG_FPS
	uint16_t fps = 0;
#endif

	matrix_init();
	ssd1306_init();
	timer0_init();
	prng_init();
	setupScreen();
#ifdef DOUBLE_BUFFER
	ssd1306_switchFrame();
	setupScreen();
#endif
	while (1) {
		start = millis();
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
					scoreScreen(score);
					sleepMode();
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
#ifdef DEBUG_FPS
			drawValue(112, 0, fps);
#else
			drawValue(112, 0, score);
#endif
#ifdef DOUBLE_BUFFER
			ssd1306_switchFrame();
#endif
			// Scan button matrix
			scanMatrix();
#ifdef DEBUG_FPS
			fps = 1000 / (millis() - start);
#endif
			// Maintain 40 fps
			while ((millis() - start) < 25);
		}
    }
}

