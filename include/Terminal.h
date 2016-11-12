//
// Created by wolverindev on 12.11.16.
//

#ifndef CXXTERMINAL_TERMINAL_H
#define CXXTERMINAL_TERMINAL_H

#define ANSI_ESC              "\x1B"
#define ANSI_BLACK            "\x1B[0;30m"
#define ANSI_BLUE             "\x1B[0;34m"
#define ANSI_GREEN            "\x1B[0;32m"
#define ANSI_CYAN             "\x1B[0;36m"
#define ANSI_RED              "\x1B[0;31m"
#define ANSI_PURPLE           "\x1B[0;35m"
#define ANSI_BROWN            "\x1B[0;33m"
#define ANSI_GRAY             "\x1B[0;37m"
#define ANSI_DARK_GREY        "\x1B[1;30m"
#define ANSI_LIGHT_BLUE       "\x1B[1;34m"
#define ANSI_LIGHT_GREEN      "\x1B[1;32m"
#define ANSI_LIGHT_CYAN       "\x1B[1;36m"
#define ANSI_LIGHT_RED        "\x1B[1;31m"
#define ANSI_LIGHT_PURPLE     "\x1B[1;35m"
#define ANSI_YELLOW           "\x1B[1;33m"
#define ANSI_WHITE            "\x1B[1;37m"

#define ANSI_BOLD             "\x1B[1m"
#define ANSI_UNDERLINE        "\x1B[4m"
#define ANSI_BLINK            "\x1B[5m"
#define ANSI_REVERSE          "\x1B[7m"
#define ANSI_INVISIABLE       "\x1B[8m"
#define ANSI_RESET            "\x1B[0m"

#include <string>

class Terminal {
    public:
        static void setup();
        static Terminal* getInstance();

};

#endif //CXXTERMINAL_TERMINAL_H
