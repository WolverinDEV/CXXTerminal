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

#define HISTORY_LINE_BUFFER_SIZE (int) 100

#include <string>
#include <vector>
#include <functional>
#include <thread>

namespace Terminal {
    typedef std::function<void(std::string, std::string, std::vector<std::string> &)> TabCompleter;

    extern void setup();

    extern void uninstall();

    extern bool isActive();

    class TerminalImpl;

    extern TerminalImpl *getInstance();

    class TerminalImpl {
        public:
            static std::string parseCharacterCodes(std::string in);
            static std::string stripCharacterCodes(std::string in);

        public:
            void redrawLine(bool lockMutex = true);

            void writeMessage(std::string message, bool noCharacterCodes = false);

            int linesAvariable();

            std::string readLine(const std::string& promt = "", int timeout = -1);

            std::vector<std::string> &getBufferedLines();

            std::string getCursorBuffer();

            void setCursorBuffer(std::string);

            int getCursorPosition();

            void setCursorPosition(int index);

            void setPromt(std::string promt);

            std::string getPromt() {
                return this->promt;
            }

            void addTabCompleter(TabCompleter *tabCompleter);

            void removeTabCompleter(TabCompleter *tabCompleter);

            int startReader();

            int stopReader();
        private:
            void printCommand(std::string command);

            int readNextByte();

            void charReaded(int character);

            std::string getNextLine();

            pthread_mutex_t readlineMutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
            std::thread *readerThread = nullptr;
            bool running = false;

            std::vector<std::string> lineBuffer;

            std::string promt = "";
            int cursorPosition = 0;
            std::vector<char> cursorBuffer;

            bool newInputTyped = true;

            std::vector<TabCompleter *> tabCompleters;
            std::vector<std::string> currentTabComplete;
            int tabCompleteIndex = 0;

            std::vector<std::string> commandHistory;
            int historyIndex = 0;
    };
}

#endif //CXXTERMINAL_TERMINAL_H
