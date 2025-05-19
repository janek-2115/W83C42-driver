#include "Arduino.h"
#pragma once

#define ESC              0x01
#define TAB              0x0F
#define CAPS_LOCK        0x3A
#define L_SHIFT          0x2A
#define R_SHIFT          0x36
#define L_CTRL           0x1D
#define L_ALT            0x38
#define SPACE            0x39
#define ENTER            0x1C
#define BACKSPACE        0x0E


class W83C42
{
private:
    char                SCAN_CODES[128] = {
        0,
        0,        // Esc
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',     // Backspace
        '\t',     // Tab
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        '\n',     // Enter
        0,        // Left Ctrl
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        '`',
        0,        // Left Shift
        '\\',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        0,        // Right Shift
        '*',      // Keypad *
        0,        // Left Alt
        ' ',      // Space
    };
    char                SCAN_CODES_SHIFTED[128] = {
        0,
        0,        // Esc
        '!',
        '@',
        '#',
        '$',
        '%',
        '^',
        '&',
        '*',
        '(',
        ')',
        '_',
        '+',
        '\b',     // Backspace
        '\t',     // Tab
        'Q',
        'W',
        'E',
        'R',
        'T',
        'Y',
        'U',
        'I',
        'O',
        'P',
        '{',
        '}',
        '\n',     // Enter
        0,        // Left Ctrl
        'A',
        'S',
        'D',
        'F',
        'G',
        'H',
        'J',
        'K',
        'L',
        ':',
        '"',
        '~',
        0,        // Left Shift
        '|',
        'Z',
        'X',
        'C',
        'V',
        'B',
        'N',
        'M',
        '<',
        '>',
        '?',
        0,        // Right Shift
        '*',      // Keypad *
        0,        // Left Alt
        ' ',      // Space
        // Add more if needed
    };
    volatile char	    buffer[128] = { 0 };
    uint8_t			    command;
    uint8_t			    status;
    uint8_t			    CS;
    uint8_t			    WR;
    uint8_t			    RD;
    uint8_t			    A2;
    uint8_t			    RST;
    uint8_t			    INT;
    uint8_t			    DATAPINS[8];
    bool                translate = true;
    volatile uint8_t    start;
    volatile uint8_t    end;
    volatile bool SHIFT_PRESSED = false;
    volatile bool CAPSLOCK_ON = false;
public:
    static W83C42* instance;
    W83C42(uint8_t D0, uint8_t D1, uint8_t D2,
        uint8_t D3, uint8_t D4, uint8_t D5,
        uint8_t D6, uint8_t D7, uint8_t CS,
        uint8_t WR, uint8_t RD, uint8_t RST,
        uint8_t A2, uint8_t INT);
    void    BUFFER_ISR();
    static void	ISR_WRAPPER();
    bool	begin();
    void	disable();
    void	enable();
    int	    readBuffer(char* buffer, int maxLen);
    uint8_t	selfTest(bool print);
    uint8_t readInputPort(bool print);
    uint8_t readStatus(bool print);
    uint8_t readCommand(bool print);
    void	sendCommand(uint8_t data);
    void	disableKeyboardInterface();
    void	enableKeyboardInterface();
    void	enableTranslation();
    void	disableTranslation();
    void	waitForIBF();
    void	waitForOBF();
    uint8_t	in(bool command, bool interrupt);
    void	out(uint8_t data, bool command);
    void	pinInput() const;
    void	pinOutput() const;
};

