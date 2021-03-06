/*
 * SSD1306 controller frame-buffer-less driver with 6x8 and 8x16 pixel fonts
 *
 * Created: 9-9-2018 14:13:39
 *  Author: Tim Dorssers
 */ 

#include "ssd1306.h"

/* Standard ASCII 6x8 font */
const uint8_t font6x8[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, //   0
	0x00, 0x00, 0x2f, 0x00, 0x00, // ! 1
	0x00, 0x07, 0x00, 0x07, 0x00, // " 2
	0x14, 0x7f, 0x14, 0x7f, 0x14, // # 3
	0x24, 0x2a, 0x7f, 0x2a, 0x12, // $ 4
	0x62, 0x64, 0x08, 0x13, 0x23, // % 5
	0x36, 0x49, 0x55, 0x22, 0x50, // & 6
	0x00, 0x05, 0x03, 0x00, 0x00, // ' 7
	0x00, 0x1c, 0x22, 0x41, 0x00, // ( 8
	0x00, 0x41, 0x22, 0x1c, 0x00, // ) 9
	0x14, 0x08, 0x3E, 0x08, 0x14, // * 10
	0x08, 0x08, 0x3E, 0x08, 0x08, // + 11
	0x00, 0x00, 0xA0, 0x60, 0x00, // , 12
	0x08, 0x08, 0x08, 0x08, 0x08, // - 13
	0x00, 0x60, 0x60, 0x00, 0x00, // . 14
	0x20, 0x10, 0x08, 0x04, 0x02, // / 15
	0x3E, 0x51, 0x49, 0x45, 0x3E, // 0 16
	0x00, 0x42, 0x7F, 0x40, 0x00, // 1 17
	0x42, 0x61, 0x51, 0x49, 0x46, // 2 18
	0x21, 0x41, 0x45, 0x4B, 0x31, // 3 19
	0x18, 0x14, 0x12, 0x7F, 0x10, // 4 20
	0x27, 0x45, 0x45, 0x45, 0x39, // 5 21
	0x3C, 0x4A, 0x49, 0x49, 0x30, // 6 22
	0x01, 0x71, 0x09, 0x05, 0x03, // 7 23
	0x36, 0x49, 0x49, 0x49, 0x36, // 8 24
	0x06, 0x49, 0x49, 0x29, 0x1E, // 9 25
	0x00, 0x36, 0x36, 0x00, 0x00, // : 26
	0x00, 0x56, 0x36, 0x00, 0x00, // ; 27
	0x08, 0x14, 0x22, 0x41, 0x00, // < 28
	0x14, 0x14, 0x14, 0x14, 0x14, // = 29
	0x00, 0x41, 0x22, 0x14, 0x08, // > 30
	0x02, 0x01, 0x51, 0x09, 0x06, // ? 31
	0x32, 0x49, 0x59, 0x51, 0x3E, // @ 32
	0x7C, 0x12, 0x11, 0x12, 0x7C, // A 33
	0x7F, 0x49, 0x49, 0x49, 0x36, // B 34
	0x3E, 0x41, 0x41, 0x41, 0x22, // C 35
	0x7F, 0x41, 0x41, 0x22, 0x1C, // D 36
	0x7F, 0x49, 0x49, 0x49, 0x41, // E 37
	0x7F, 0x09, 0x09, 0x09, 0x01, // F 38
	0x3E, 0x41, 0x49, 0x49, 0x7A, // G 39
	0x7F, 0x08, 0x08, 0x08, 0x7F, // H 40
	0x00, 0x41, 0x7F, 0x41, 0x00, // I 41
	0x20, 0x40, 0x41, 0x3F, 0x01, // J 42
	0x7F, 0x08, 0x14, 0x22, 0x41, // K 43
	0x7F, 0x40, 0x40, 0x40, 0x40, // L 44
	0x7F, 0x02, 0x0C, 0x02, 0x7F, // M 45
	0x7F, 0x04, 0x08, 0x10, 0x7F, // N 46
	0x3E, 0x41, 0x41, 0x41, 0x3E, // O 47
	0x7F, 0x09, 0x09, 0x09, 0x06, // P 48
	0x3E, 0x41, 0x51, 0x21, 0x5E, // Q 49
	0x7F, 0x09, 0x19, 0x29, 0x46, // R 50
	0x46, 0x49, 0x49, 0x49, 0x31, // S 51
	0x01, 0x01, 0x7F, 0x01, 0x01, // T 52
	0x3F, 0x40, 0x40, 0x40, 0x3F, // U 53
	0x1F, 0x20, 0x40, 0x20, 0x1F, // V 54
	0x3F, 0x40, 0x38, 0x40, 0x3F, // W 55
	0x63, 0x14, 0x08, 0x14, 0x63, // X 56
	0x07, 0x08, 0x70, 0x08, 0x07, // Y 57
	0x61, 0x51, 0x49, 0x45, 0x43, // Z 58
	0x00, 0x7F, 0x41, 0x41, 0x00, // [ 59
	0x02, 0x04, 0x08, 0x10, 0x20, // \ 60
	0x00, 0x41, 0x41, 0x7F, 0x00, // ] 61
	0x04, 0x02, 0x01, 0x02, 0x04, // ^ 62
	0x40, 0x40, 0x40, 0x40, 0x40, // _ 63
	0x00, 0x01, 0x02, 0x04, 0x00, // ' 64
	0x20, 0x54, 0x54, 0x54, 0x78, // a 65
	0x7F, 0x48, 0x44, 0x44, 0x38, // b 66
	0x38, 0x44, 0x44, 0x44, 0x20, // c 67
	0x38, 0x44, 0x44, 0x48, 0x7F, // d 68
	0x38, 0x54, 0x54, 0x54, 0x18, // e 69
	0x08, 0x7E, 0x09, 0x01, 0x02, // f 70
	0x18, 0xA4, 0xA4, 0xA4, 0x7C, // g 71
	0x7F, 0x08, 0x04, 0x04, 0x78, // h 72
	0x00, 0x44, 0x7D, 0x40, 0x00, // i 73
	0x40, 0x80, 0x84, 0x7D, 0x00, // j 74
	0x7F, 0x10, 0x28, 0x44, 0x00, // k 75
	0x00, 0x41, 0x7F, 0x40, 0x00, // l 76
	0x7C, 0x04, 0x18, 0x04, 0x78, // m 77
	0x7C, 0x08, 0x04, 0x04, 0x78, // n 78
	0x38, 0x44, 0x44, 0x44, 0x38, // o 79
	0xFC, 0x24, 0x24, 0x24, 0x18, // p 80
	0x18, 0x24, 0x24, 0x18, 0xFC, // q 81
	0x7C, 0x08, 0x04, 0x04, 0x08, // r 82
	0x48, 0x54, 0x54, 0x54, 0x20, // s 83
	0x04, 0x3F, 0x44, 0x40, 0x20, // t 84
	0x3C, 0x40, 0x40, 0x20, 0x7C, // u 85
	0x1C, 0x20, 0x40, 0x20, 0x1C, // v 86
	0x3C, 0x40, 0x30, 0x40, 0x3C, // w 87
	0x44, 0x28, 0x10, 0x28, 0x44, // x 88
	0x1C, 0xA0, 0xA0, 0xA0, 0x7C, // y 89
	0x44, 0x64, 0x54, 0x4C, 0x44, // z 90
	0x08, 0x36, 0x41, 0x41, 0x00, // { 91
	0x00, 0x00, 0x7F, 0x00, 0x00, // | 92
	0x00, 0x41, 0x41, 0x36, 0x08, // } 93
	0x08, 0x04, 0x08, 0x10, 0x08, // ~ 94
};

/* Standard ASCII 8x16 font */
const uint8_t font8x16[] PROGMEM = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //   0
	0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00, // ! 1
	0x00,0x10,0x0C,0x06,0x10,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // " 2
	0x40,0xC0,0x78,0x40,0xC0,0x78,0x40,0x00,0x04,0x3F,0x04,0x04,0x3F,0x04,0x04,0x00, // # 3
	0x00,0x70,0x88,0xFC,0x08,0x30,0x00,0x00,0x00,0x18,0x20,0xFF,0x21,0x1E,0x00,0x00, // $ 4
	0xF0,0x08,0xF0,0x00,0xE0,0x18,0x00,0x00,0x00,0x21,0x1C,0x03,0x1E,0x21,0x1E,0x00, // % 5
	0x00,0xF0,0x08,0x88,0x70,0x00,0x00,0x00,0x1E,0x21,0x23,0x24,0x19,0x27,0x21,0x10, // & 6
	0x10,0x16,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ' 7
	0x00,0x00,0x00,0xE0,0x18,0x04,0x02,0x00,0x00,0x00,0x00,0x07,0x18,0x20,0x40,0x00, // ( 8
	0x00,0x02,0x04,0x18,0xE0,0x00,0x00,0x00,0x00,0x40,0x20,0x18,0x07,0x00,0x00,0x00, // ) 9
	0x40,0x40,0x80,0xF0,0x80,0x40,0x40,0x00,0x02,0x02,0x01,0x0F,0x01,0x02,0x02,0x00, // * 10
	0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x1F,0x01,0x01,0x01,0x00, // + 11
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xB0,0x70,0x00,0x00,0x00,0x00,0x00, // , 12
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01, // - 13
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00, // . 14
	0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x04,0x00,0x60,0x18,0x06,0x01,0x00,0x00,0x00, // / 15
	0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00, // 0 16
	0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00, // 1 17
	0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00, // 2 18
	0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00, // 3 19
	0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00, // 4 20
	0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00, // 5 21
	0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00, // 6 22
	0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00, // 7 23
	0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00, // 8 24
	0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00, // 9 25
	0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00, // : 26
	0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x00, // ; 27
	0x00,0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x00, // < 28
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00, // = 29
	0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,0x01,0x00, // > 30
	0x00,0x70,0x48,0x08,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x30,0x36,0x01,0x00,0x00, // ? 31
	0xC0,0x30,0xC8,0x28,0xE8,0x10,0xE0,0x00,0x07,0x18,0x27,0x24,0x23,0x14,0x0B,0x00, // @ 32
	0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20, // A 33
	0x08,0xF8,0x88,0x88,0x88,0x70,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00, // B 34
	0xC0,0x30,0x08,0x08,0x08,0x08,0x38,0x00,0x07,0x18,0x20,0x20,0x20,0x10,0x08,0x00, // C 35
	0x08,0xF8,0x08,0x08,0x08,0x10,0xE0,0x00,0x20,0x3F,0x20,0x20,0x20,0x10,0x0F,0x00, // D 36
	0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x20,0x23,0x20,0x18,0x00, // E 37
	0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x00,0x03,0x00,0x00,0x00, // F 38
	0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00,0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00, // G 39
	0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x20,0x3F,0x21,0x01,0x01,0x21,0x3F,0x20, // H 40
	0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00, // I 41
	0x00,0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,0x00, // J 42
	0x08,0xF8,0x88,0xC0,0x28,0x18,0x08,0x00,0x20,0x3F,0x20,0x01,0x26,0x38,0x20,0x00, // K 43
	0x08,0xF8,0x08,0x00,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x20,0x30,0x00, // L 44
	0x08,0xF8,0xF8,0x00,0xF8,0xF8,0x08,0x00,0x20,0x3F,0x00,0x3F,0x00,0x3F,0x20,0x00, // M 45
	0x08,0xF8,0x30,0xC0,0x00,0x08,0xF8,0x08,0x20,0x3F,0x20,0x00,0x07,0x18,0x3F,0x00, // N 46
	0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x10,0x20,0x20,0x20,0x10,0x0F,0x00, // O 47
	0x08,0xF8,0x08,0x08,0x08,0x08,0xF0,0x00,0x20,0x3F,0x21,0x01,0x01,0x01,0x00,0x00, // P 48
	0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x18,0x24,0x24,0x38,0x50,0x4F,0x00, // Q 49
	0x08,0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x00,0x03,0x0C,0x30,0x20, // R 50
	0x00,0x70,0x88,0x08,0x08,0x08,0x38,0x00,0x00,0x38,0x20,0x21,0x21,0x22,0x1C,0x00, // S 51
	0x18,0x08,0x08,0xF8,0x08,0x08,0x18,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00, // T 52
	0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00, // U 53
	0x08,0x78,0x88,0x00,0x00,0xC8,0x38,0x08,0x00,0x00,0x07,0x38,0x0E,0x01,0x00,0x00, // V 54
	0xF8,0x08,0x00,0xF8,0x00,0x08,0xF8,0x00,0x03,0x3C,0x07,0x00,0x07,0x3C,0x03,0x00, // W 55
	0x08,0x18,0x68,0x80,0x80,0x68,0x18,0x08,0x20,0x30,0x2C,0x03,0x03,0x2C,0x30,0x20, // X 56
	0x08,0x38,0xC8,0x00,0xC8,0x38,0x08,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00, // Y 57
	0x10,0x08,0x08,0x08,0xC8,0x38,0x08,0x00,0x20,0x38,0x26,0x21,0x20,0x20,0x18,0x00, // Z 58
	0x00,0x00,0x00,0xFE,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x00, // [ 59
	0x00,0x0C,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x38,0xC0,0x00, // \ 60
	0x00,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x7F,0x00,0x00,0x00, // ] 61
	0x00,0x00,0x04,0x02,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ^ 62
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, // _ 63
	0x00,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ` 64
	0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x19,0x24,0x22,0x22,0x22,0x3F,0x20, // a 65
	0x08,0xF8,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x3F,0x11,0x20,0x20,0x11,0x0E,0x00, // b 66
	0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x00,0x0E,0x11,0x20,0x20,0x20,0x11,0x00, // c 67
	0x00,0x00,0x00,0x80,0x80,0x88,0xF8,0x00,0x00,0x0E,0x11,0x20,0x20,0x10,0x3F,0x20, // d 68
	0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x22,0x22,0x22,0x22,0x13,0x00, // e 69
	0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x18,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00, // f 70
	0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x6B,0x94,0x94,0x94,0x93,0x60,0x00, // g 71
	0x08,0xF8,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20, // h 72
	0x00,0x80,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00, // i 73
	0x00,0x00,0x00,0x80,0x98,0x98,0x00,0x00,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00, // j 74
	0x08,0xF8,0x00,0x00,0x80,0x80,0x80,0x00,0x20,0x3F,0x24,0x02,0x2D,0x30,0x20,0x00, // k 75
	0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00, // l 76
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x3F,0x20,0x00,0x3F,0x20,0x00,0x3F, // m 77
	0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20, // n 78
	0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00, // o 79
	0x80,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0xFF,0xA1,0x20,0x20,0x11,0x0E,0x00, // p 80
	0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x0E,0x11,0x20,0x20,0xA0,0xFF,0x80, // q 81
	0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x20,0x20,0x3F,0x21,0x20,0x00,0x01,0x00, // r 82
	0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x33,0x24,0x24,0x24,0x24,0x19,0x00, // s 83
	0x00,0x80,0x80,0xE0,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00,0x00, // t 84
	0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x1F,0x20,0x20,0x20,0x10,0x3F,0x20, // u 85
	0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x00,0x01,0x0E,0x30,0x08,0x06,0x01,0x00, // v 86
	0x80,0x80,0x00,0x80,0x00,0x80,0x80,0x80,0x0F,0x30,0x0C,0x03,0x0C,0x30,0x0F,0x00, // w 87
	0x00,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x31,0x2E,0x0E,0x31,0x20,0x00, // x 88
	0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,0x81,0x8E,0x70,0x18,0x06,0x01,0x00, // y 89
	0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x21,0x30,0x2C,0x22,0x21,0x30,0x00, // z 90
	0x00,0x00,0x00,0x00,0x80,0x7C,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x3F,0x40,0x40, // { 91
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00, // | 92
	0x00,0x02,0x02,0x7C,0x80,0x00,0x00,0x00,0x00,0x40,0x40,0x3F,0x00,0x00,0x00,0x00, // } 93
	0x00,0x06,0x01,0x01,0x02,0x02,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ~ 94
};

const uint8_t ssd1306_init_sequence[] PROGMEM = {	// Initialization Sequence
	//	0xAE,			// Display OFF (sleep mode)
	//	0x20, 0b10,		// Set Memory Addressing Mode
					// 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;
					// 10=Page Addressing Mode (RESET); 11=Invalid
	//	0xB0,			// Set Page Start Address for Page Addressing Mode, 0-7
	0xC8,			// Set COM Output Scan Direction
	//	0x00,			// ---set low column address
	//	0x10,			// ---set high column address
	//	0x40,			// --set start line address
	//	0x81, 0x7F,		// Set contrast control register
	0xA1,			// Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	//	0xA6,			// Set display mode. A6=Normal; A7=Inverse
	0xA8, 0x1F,		// Set multiplex ratio(1 to 64)
	//	0xA4,			// Output RAM to Display
					// 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
	//	0xD3, 0x00,		// Set display offset. 00 = no offset
	//	0xD5, 0x80,		// --set display clock divide ratio/oscillator frequency
	//	0xD9, 0x22,		// Set pre-charge period
	0xDA, 0x02,		// Set com pins hardware configuration
	//	0xDB, 0x20,		// --set vcomh 0x20 = 0.77xVcc
	0x8D, 0x14		// Set DC-DC enable
};

uint8_t oledX = 0, oledY = 0;
uint8_t renderingFrame = 0xB0, drawingFrame = 0x40;

void ssd1306_send_command_start(void) {
	i2c_start(SSD1306_ADDR + I2C_WRITE);
	i2c_write(SSD1306_COMMAND);
}

void ssd1306_init(void) {
	ssd1306_send_command_start();
	for (uint8_t i = 0; i < sizeof(ssd1306_init_sequence); i++) {
		i2c_write(pgm_read_byte(&ssd1306_init_sequence[i]));
	}
	i2c_stop();
	//ssd1306_set_com_output_direction(1);
	//ssd1306_set_segment_remap(1);
	//ssd1306_set_multiplex_ratio(32);
	//ssd1306_set_com_pins_hardware_configuration(0, 0);
	//ssd1306_enable_charge_pump();
}

void ssd1306_send_command(uint8_t command) {
	ssd1306_send_command_start();
	i2c_write(command);
	i2c_stop();
}

void ssd1306_send_data_start(void) {
	i2c_start(SSD1306_ADDR + I2C_WRITE);
	i2c_write(SSD1306_DATA);
}

void ssd1306_set_cursor(uint8_t x, uint8_t y) {
	ssd1306_send_command_start();
	i2c_write(renderingFrame | (y & 0x07));
	i2c_write(0x10 | ((x & 0xf0) >> 4));
	i2c_write(x & 0x0f);
	i2c_stop();
	oledX = x;
	oledY = y;
}

void ssd1306_fill_length(uint8_t fill, uint8_t length) {
	oledX += length;
	ssd1306_send_data_start();
	do {
		i2c_write(fill);
	} while (--length);
	i2c_stop();
}

void ssd1306_fill_to_eol(uint8_t fill) {
	ssd1306_fill_length(fill, 128 - oledX);
}

void ssd1306_fill(uint8_t fill) {
	for (uint8_t i = 0; i < SSD1306_PAGES; i++) {
		ssd1306_set_cursor(0, i);
		ssd1306_fill_to_eol(fill);
	}
	ssd1306_set_cursor(0, 0);
}

void ssd1306_clear(void) {
	for (uint8_t y = 0; y < SSD1306_PAGES; y++) {
		ssd1306_set_cursor(0, y);
		ssd1306_send_data_start();
		for (uint8_t i = 128; --i > 0; ) {
			i2c_write(0x00);
		}
		i2c_stop();
	}
	ssd1306_set_cursor(0, 0);
}

// height in pages (8 pixels)
void ssd1306_new_line(uint8_t fontHeight) {
	oledY+=fontHeight;
	if (oledY > SSD1306_PAGES - fontHeight) {
		oledY = SSD1306_PAGES - fontHeight;
	}
	ssd1306_set_cursor(0, oledY);
}

void ssd1306_bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bitmap[]) {
	uint16_t j = 0;
	for (uint8_t y = y0; y < y1; y++) {
		ssd1306_set_cursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t x = x0; x < x1; x++) {
			i2c_write(bitmap[j++]);
		}
		i2c_stop();
	}
	ssd1306_set_cursor(0, 0);
}

void ssd1306_bitmap_p(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
	uint16_t j = 0;
	for (uint8_t y = y0; y < y1; y++) {
		ssd1306_set_cursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t x = x0; x < x1; x++) {
			i2c_write(pgm_read_byte(&bitmap[j++]));
		}
		i2c_stop();
	}
	ssd1306_set_cursor(0, 0);
}

void ssd1306_char_font6x8(uint8_t c) {
	if (c == '\r')
		return;
	if (c == '\n') {
		ssd1306_new_line(1);
		return;
	}
	if (oledX > 122) {
		ssd1306_new_line(1);
	}
	ssd1306_send_data_start();
	uint16_t offset = ((uint16_t)c - 32) * 5;
	i2c_write(0x00);
	for (uint8_t i = 0; i < 5; i++) {
		i2c_write(pgm_read_byte(&font6x8[offset++]));
	}
	i2c_stop();
	oledX+=6;
}

void ssd1306_string_font6x8(uint8_t *s) {
	while (*s) {
		ssd1306_char_font6x8(*s++);
	}
}

void ssd1306_string_font6x8_p(const uint8_t *s) {
	uint8_t c;
	while ((c = pgm_read_byte(s++))) {
		ssd1306_char_font6x8(c);
	}
}

void ssd1306_char_font8x16(uint8_t c) {
	if (c == '\r')
		return;
	if (c == '\n') {
		ssd1306_new_line(2);
		return;
	}
	if (oledX > 120) {
		ssd1306_new_line(2);
	}
	uint16_t offset = ((uint16_t)c - 32) * 16;
	uint8_t line = 2;
	do
	{
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < 8; i++) {
			i2c_write(pgm_read_byte(&font8x16[offset++]));
		}
		i2c_stop();
		if (line > 1) {
			ssd1306_set_cursor(oledX, oledY + 1);
		}
		else {
			ssd1306_set_cursor(oledX + 8, oledY - 1);
		}
	}
	while (--line);
}

void ssd1306_string_font8x16(uint8_t *s) {
	while (*s) {
		ssd1306_char_font8x16(*s++);
	}
}

void ssd1306_string_font8x16_p(const uint8_t *s) {
	uint8_t c;
	while ((c = pgm_read_byte(s++))) {
		ssd1306_char_font8x16(c);
	}
}

void ssd1306_put_pixel(uint8_t x, uint8_t y) {
	ssd1306_set_cursor(x, y / 8);
	ssd1306_send_data_start();
	i2c_write(1 << y % 8);
	i2c_stop();
}

void ssd1306_put_pixels(uint8_t x, uint8_t y, uint8_t pixels) {
	ssd1306_set_cursor(x, y / 8);
	ssd1306_send_data_start();
	i2c_write(pixels);
	i2c_stop();
}

void ssd1306_draw_hline(uint8_t y) {
	for (uint8_t i = 0; i < 128; i++) {
		ssd1306_put_pixel(i, y);
	}
}

void ssd1306_draw_vline(uint8_t x) {
	for (uint8_t i = 0; i < SSD1306_PAGES; i++) {
		ssd1306_set_cursor(x, i);
		ssd1306_send_data_start();
		i2c_write(0xFF);
		i2c_stop();
	}
}

// 1. Fundamental Command Table

void ssd1306_set_contrast(uint8_t contrast) {
	ssd1306_send_command_start();
	i2c_write(0x81);
	i2c_write(contrast);
	i2c_stop();
}

// Double Buffering Commands

void ssd1306_switch_render_frame(void) {
	renderingFrame ^= 0x04;
}

void ssd1306_switch_display_frame(void) {
	drawingFrame ^= 0x20;
	ssd1306_send_command(drawingFrame);
}

void ssd1306_switchFrame(void) {
	//ssd1306_switch_display_frame();
	//ssd1306_switch_render_frame();
	drawingFrame ^= 0x20;
	ssd1306_send_command(drawingFrame);
	renderingFrame ^= 0x04;
}

uint8_t ssd1306_current_render_frame(void) {
	return (renderingFrame >> 2) & 0x01;
}

uint8_t ssd1306_current_display_frame(void) {
	return (drawingFrame >> 5) & 0x01;
}

// 2. Scrolling Command Table

void ssd1306_scroll_right(uint8_t startPage, uint8_t interval, uint8_t endPage) {
	ssd1306_send_command_start();
	i2c_write(0x26);
	i2c_write(0x00);
	i2c_write(startPage);
	i2c_write(interval);
	i2c_write(endPage);
	i2c_write(0x00);
	i2c_write(0xFF);
	i2c_stop();
}

void ssd1306_scroll_left(uint8_t startPage, uint8_t interval, uint8_t endPage) {
	ssd1306_send_command_start();
	i2c_write(0x27);
	i2c_write(0x00);
	i2c_write(startPage);
	i2c_write(interval);
	i2c_write(endPage);
	i2c_write(0x00);
	i2c_write(0xFF);
	i2c_stop();
}

void ssd1306_scroll_right_vertical_offset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset) {
	ssd1306_send_command_start();
	i2c_write(0x29);
	i2c_write(0x00);
	i2c_write(startPage);
	i2c_write(interval);
	i2c_write(endPage);
	i2c_write(offset);
	i2c_stop();
}

void ssd1306_scroll_left_vertical_offset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset) {
	ssd1306_send_command_start();
	i2c_write(0x2A);
	i2c_write(0x00);
	i2c_write(startPage);
	i2c_write(interval);
	i2c_write(endPage);
	i2c_write(offset);
	i2c_stop();
}

void ssd1306_set_vertical_scroll_area(uint8_t top, uint8_t rows) {
	ssd1306_send_command_start();
	i2c_write(0xA3);
	i2c_write(top);
	i2c_write(rows);
	i2c_stop();
}

// 3. Addressing Setting Command Table

void ssd1306_set_column_start_address(uint8_t startAddress) {
	ssd1306_send_command_start();
	i2c_write(startAddress & 0x0F);
	i2c_write(startAddress >> 4);
	i2c_stop();
}

void ssd1306_set_memory_addressing_mode(uint8_t mode) {
	ssd1306_send_command_start();
	i2c_write(0x20);
	i2c_write(mode & 0x03);
	i2c_stop();
}

void ssd1306_set_column_address(uint8_t startAddress, uint8_t endAddress) {
	ssd1306_send_command_start();
	i2c_write(0x21);
	i2c_write(startAddress & 0x7F);
	i2c_write(endAddress & 0x7F);
	i2c_stop();
}

void ssd1306_set_page_address(uint8_t startPage, uint8_t endPage) {
	ssd1306_send_command_start();
	i2c_write(0x22);
	i2c_write(startPage & 0x07);
	i2c_write(endPage & 0x07);
	i2c_stop();
}

void ssd1306_set_page_start_address(uint8_t startPage) {
	ssd1306_send_command(0xB0 | (startPage & 0x07));
}

// 4. Hardware Configuration (Panel resolution and layout related) Command Table

void ssd1306_set_display_start_line(uint8_t startLine) {
	ssd1306_send_command(0x40 | (startLine & 0x3F));
}

void ssd1306_set_segment_remap(uint8_t remap) {
	ssd1306_send_command(0xA0 | (remap & 0x01));
}

void ssd1306_set_multiplex_ratio(uint8_t mux) {
	ssd1306_send_command_start();
	i2c_write(0xA8);
	i2c_write((mux - 1) & 0x3F);
	i2c_stop();
}

void ssd1306_set_com_output_direction(uint8_t direction) {
	ssd1306_send_command(0xC0 | ((direction & 0x01)<<3));
}

void ssd1306_set_display_offset(uint8_t offset) {
	ssd1306_send_command_start();
	i2c_write(0xD3);
	i2c_write(offset & 0x3F);
	i2c_stop();
}

void ssd1306_set_com_pins_hardware_configuration(uint8_t alternative, uint8_t enableLeftRightRemap) {
	ssd1306_send_command_start();
	i2c_write(0xDA);
	i2c_write(((enableLeftRightRemap & 0x01) << 5) | ((alternative & 0x01) << 4) | 0x02);
	i2c_stop();
}

// 5. Timing and Driving Scheme Setting Command table

void ssd1306_set_display_clock(uint8_t divideRatio, uint8_t oscillatorFrequency) {
	ssd1306_send_command_start();
	i2c_write(0xD5);
	i2c_write(((oscillatorFrequency & 0x0F) << 4) | ((divideRatio -1) & 0x0F));
	i2c_stop();
}

void ssd1306_set_precharge_period(uint8_t phaseOnePeriod, uint8_t phaseTwoPeriod) {
	ssd1306_send_command_start();
	i2c_write(0xD9);
	i2c_write(((phaseTwoPeriod & 0x0F) << 4) | (phaseOnePeriod & 0x0F));
	i2c_stop();
}

void ssd1306_set_vcomh_deselect_level(uint8_t level) {
	ssd1306_send_command_start();
	i2c_write(0xDB);
	i2c_write((level & 0x07) << 4);
	i2c_stop();
}

// 6. Advance Graphic Command table

void ssd1306_fade_out(uint8_t interval) {
	ssd1306_send_command_start();
	i2c_write(0x23);
	i2c_write((0x20 | (interval & 0x0F)));
	i2c_stop();
}

void ssd1306_blink(uint8_t interval) {
	ssd1306_send_command_start();
	i2c_write(0x23);
	i2c_write((0x30 | (interval & 0x0F)));
	i2c_stop();
}

void ssd1306_disable_fade_out_and_blinking(void) {
	ssd1306_send_command_start();
	i2c_write(0x23);
	i2c_write(0x00);
	i2c_stop();
}

void ssd1306_enable_zoom_in(void) {
	ssd1306_send_command_start();
	i2c_write(0xD6);
	i2c_write(0x01);
	i2c_stop();
}

void ssd1306_disable_zoom_in(void) {
	ssd1306_send_command_start();
	i2c_write(0xD6);
	i2c_write(0x00);
	i2c_stop();
}

// Charge Pump Settings

void ssd1306_enable_charge_pump(void) {
	ssd1306_send_command_start();
	i2c_write(0x8D);
	i2c_write(0x14);
	i2c_stop();
}

void ssd1306_disable_charge_pump(void) {
	ssd1306_send_command_start();
	i2c_write(0x8D);
	i2c_write(0x10);
	i2c_stop();
}
