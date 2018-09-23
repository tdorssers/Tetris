/*
 * ssd1306.h
 *
 * Created: 9-9-2018 14:13:56
 *  Author: Tim Dorssers
 */ 


#ifndef SSD1306_H_
#define SSD1306_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "i2cmaster.h"

#define SSD1306_PAGES 4
#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40
#define SSD1306_ADDR (0x3C*2)	// Slave address

void ssd1306_send_command_start(void);
void ssd1306_init(void);
void ssd1306_send_command(uint8_t command);
void ssd1306_send_data_start(void);
void ssd1306_set_cursor(uint8_t x, uint8_t y);
void ssd1306_fill_length(uint8_t fill, uint8_t length);
void ssd1306_fill_to_eol(uint8_t fill);
#define ssd1306_clear_to_eol() ssd1306_fill_to_eol(0x00)
void ssd1306_fill(uint8_t fill);
#define ssd1306_clear() ssd1306_fill(0x00)
void ssd1306_new_line(uint8_t fontHeight); // height in pages (8 pixels)
void ssd1306_bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bitmap[]);
void ssd1306_bitmap_p(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
void ssd1306_char_font6x8(uint8_t c);
void ssd1306_string_font6x8(uint8_t *s);
void ssd1306_string_font6x8_p(const uint8_t *s);
void ssd1306_char_font8x16(uint8_t c);
void ssd1306_string_font8x16(uint8_t *s);
void ssd1306_string_font8x16_p(const uint8_t *s);
void ssd1306_put_pixel(uint8_t x, uint8_t y);
void ssd1306_put_pixels(uint8_t x, uint8_t y, uint8_t pixels);
void ssd1306_draw_hline(uint8_t y);
void ssd1306_draw_vline(uint8_t x);
// 1. Fundamental Command Table
void ssd1306_set_contrast(uint8_t contrast);
void ssd1306_set_entire_display_on(uint8_t enable);
void ssd1306_set_inverse(uint8_t enable);
#define ssd1306_off() ssd1306_send_command(0xAE)
#define ssd1306_on() ssd1306_send_command(0xAF)
// Double Buffering Commands
void ssd1306_switch_render_frame(void);
void ssd1306_switch_display_frame(void);
void ssd1306_switchFrame(void);
uint8_t ssd1306_current_render_frame(void);
uint8_t ssd1306_current_display_frame(void);
// 2. Scrolling Command Table
void ssd1306_scroll_right(uint8_t startPage, uint8_t interval, uint8_t endPage);
void ssd1306_scroll_left(uint8_t startPage, uint8_t interval, uint8_t endPage);
void ssd1306_scroll_right_vertical_offset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset);
void ssd1306_scroll_left_vertical_offset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset);
#define ssd1306_deactivate_scroll() ssd1306_send_command(0x2E)
#define ssd1306_activate_scroll() ssd1306_send_command(0x2F)
void ssd1306_set_vertical_scroll_area(uint8_t top, uint8_t rows);
// 3. Addressing Setting Command Table
void ssd1306_set_column_start_address(uint8_t startAddress);
void ssd1306_set_memory_addressing_mode(uint8_t mode);
void ssd1306_set_column_address(uint8_t startAddress, uint8_t endAddress);
void ssd1306_set_page_address(uint8_t startPage, uint8_t endPage);
void ssd1306_set_page_start_address(uint8_t startPage);
// 4. Hardware Configuration (Panel resolution and layout related) Command Table
void ssd1306_set_display_start_line(uint8_t startLine);
void ssd1306_set_segment_remap(uint8_t remap);
void ssd1306_set_multiplex_ratio(uint8_t mux);
void ssd1306_set_com_output_direction(uint8_t direction);
void ssd1306_set_display_offset(uint8_t offset);
void ssd1306_set_com_pins_hardware_configuration(uint8_t alternative, uint8_t enableLeftRightRemap);
// 5. Timing and Driving Scheme Setting Command table
void ssd1306_set_display_clock(uint8_t divideRatio, uint8_t oscillatorFrequency);
void ssd1306_set_precharge_period(uint8_t phaseOnePeriod, uint8_t phaseTwoPeriod);
void ssd1306_set_vcomh_deselect_level(uint8_t level);
#define ssd1306_nop() ssd1306_send_command(0xE3)
// 6. Advance Graphic Command table
void ssd1306_fade_out(uint8_t interval);
void ssd1306_blink(uint8_t interval);
void ssd1306_disable_fade_out_and_blinking(void);
void ssd1306_enable_zoom_in(void);
void ssd1306_disable_zoom_in(void);
// Charge Pump Settings
void ssd1306_enable_charge_pump(void);
void ssd1306_disable_charge_pump(void);

#endif /* SSD1306_H_ */