#pragma once

#include "AnsiCodes.h"
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <deque>

#ifdef USE_LIBEVENT
    #include <event.h>
#endif


namespace terminal {
    typedef std::function<void(std::string, std::string, std::vector<std::string> &)> TabCompleter;

#ifdef SUPPORT_LEGACY
    extern void setup(){ install(); }
    extern bool isActive(){ return active(); }
    extern Impl* getInstance(){ return instance(); }
#endif

    extern void install();
    extern void uninstall();
    extern bool active();

    class impl;
    extern impl* instance();

    extern std::string parseCharacterCodes(std::string in, std::string characterCode = "§");
    extern std::string stripCharacterCodes(std::string in, std::string characterCode = "§");

    class impl {
        public:
            void redrawLine(bool lockMutex = true);

            void writeMessage(std::string message, bool noCharacterCodes = false);

            size_t linesAvailable();

            std::string readLine(const std::string& prompt = "", std::chrono::time_point<std::chrono::system_clock> timeout = std::chrono::time_point<std::chrono::system_clock>());

            std::string getCursorBuffer();
            void cursorPosition(std::string);

            size_t cursorPosition();
            void setCursorPosition(size_t index);

            void setPrompt(std::string prompt);
            std::string getPrompt() { return this->prompt; }

            void addTabCompleter(TabCompleter *tabCompleter);
            void removeTabCompleter(TabCompleter *tabCompleter);

            int startReader();
            int stopReader();
        private:
            void printAnsiCommand(std::string command);

#ifndef USE_LIBEVENT
            int readNextByte();
#else
            void handleInput(int, short, void*);
#endif
            std::string rdbuf;
            std::mutex rdbufLock;
            bool handleRead(); //if true then trigger again

            std::string getNextLine();

            std::mutex bufferMutex;
            std::mutex mutex;

            std::thread* readerThread = nullptr;
#ifdef USE_LIBEVENT
            event_base* eventLoop = nullptr;
            event* readEvent = nullptr;
#endif
            bool running = false;

            std::deque<std::string> lineBuffer;

            std::string prompt = "";
            size_t _cursorPosition = 0;
            std::vector<char> cursorBuffer;

            bool newInputTyped = true;

            std::mutex tabCompleterLock;
            std::vector<TabCompleter *> tabCompleters;
            std::vector<std::string> currentTabComplete;
            int tabCompleteIndex = 0;

            std::vector<std::string> commandHistory;
            size_t maxHistorySize = 100;
            int historyIndex = 0;
    };
}

#ifdef SUPPORT_LEGACY
    namespace Terminal = terminal;
#endif