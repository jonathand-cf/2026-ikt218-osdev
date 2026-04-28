#include "terminal.h"
#include <libc/stdint.h>
#include <libc/system.h>

//screen is 80 columns x 25 rows (vga text mode), each cell is 2 bytes (character + attribute/color)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((uint16_t*)0xB8000) //memory address for writing to screen

static size_t terminal_row;
static size_t terminal_col;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

//pack character and color into 16-bit VGA entry
static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

//background upper 4 bits and foreground lower 4 bits
static inline uint8_t vga_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

// Updates the hardware cursor.
static void terminal_move_cursor()
{
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_col = 0;
    terminal_color = vga_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    terminal_buffer = VGA_BUFFER;

    //fill all cells with blank space to clear screen
    for (size_t row = 0; row < VGA_HEIGHT; row++) {
        for (size_t col = 0; col < VGA_WIDTH; col++) {
            //converts row and column into one number (VGA buffer is a cells list not grid)
            terminal_buffer[row * VGA_WIDTH + col] = vga_entry(' ', terminal_color);
        }
    }
    terminal_move_cursor();
}

void terminal_set_color(uint8_t fg, uint8_t bg) {
    terminal_color = vga_color(fg, bg);
}

//scrolls screen up one row when cursor reaches bottom
//by copying each row one position up and clearing last row for typing space
static void terminal_scroll(void) {
    for (size_t row = 1; row < VGA_HEIGHT; row++) {
        for (size_t col = 0; col < VGA_WIDTH; col++) {
            terminal_buffer[(row - 1) * VGA_WIDTH + col] =
                terminal_buffer[row * VGA_WIDTH + col];
        }
    }
    //clear the last row
    for (size_t col = 0; col < VGA_WIDTH; col++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] =
            vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

//writes a single character to the screen at cursors current position
void terminal_write_char(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        terminal_move_cursor();
        return;
    } else if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_col = VGA_WIDTH - 1;
        }
        // Clear the character at the new position
        terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] = vga_entry(' ', terminal_color);
        terminal_move_cursor();
        return;
    }

    terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
        vga_entry(c, terminal_color);

    terminal_col++;
    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
    terminal_move_cursor();
}

//writes a string to the screen one character at a time
void terminal_write(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_write_char(str[i]);
    }
}