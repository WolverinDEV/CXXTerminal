#include "../include/Terminal.h"

#include <sstream>
#include <iostream>
#include <csignal>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <fcntl.h>

#ifndef WIN32
	#include <unistd.h>
	#include <termios.h>
#else
	typedef int64_t ssize_t;
#endif

using namespace std;
using namespace std::chrono;
using namespace terminal;

/**
 * Quick terminal for instand access
 */
void writeMessage(const std::string &message){
    if(active())
        instance()->writeMessage(message);
    else
        std::cout << message << std::endl << flush;
}
bool isTerminalEnabled(){
    return instance() != nullptr;
}

/**
 * Terminal class
 */

#ifndef WIN32
	struct termios orig_termios;
#endif

impl* terminalInstance = nullptr;


terminal::impl* terminal::instance() {
    return terminalInstance;
}

void removeNonblock(){
#ifndef TERMINAL_NON_BLOCK
#ifndef WIN32
    tcsetattr(0, TCSANOW, &orig_termios);
    std::cout << ANSI_RESET << flush;
#endif
#endif
}

void initNonblock(){
#ifndef TERMINAL_NON_BLOCK
#ifndef WIN32
    termios new_termios{};

    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    new_termios.c_lflag &= ~ICANON;
    new_termios.c_lflag &= ~ECHO;

    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &new_termios);

    atexit(removeNonblock);
#endif
#endif
}

void terminal::install() {
    assert(!terminalInstance);

    terminalInstance = new impl();
    initNonblock();
    terminalInstance->startReader();
}

void terminal::uninstall() {
    assert(terminalInstance);

    terminalInstance->stopReader();
    if(!terminalInstance->getPrompt().empty()) cout << "\r" << flush;
    delete terminalInstance;
    terminalInstance = nullptr;
    removeNonblock();
}

bool terminal::active() {
    return terminalInstance != nullptr;
}

impl::impl() {
#if WIN32
	this->console_handle = GetStdHandle(STD_INPUT_HANDLE);
    this->console_own_handle = false;
#endif
}

impl::~impl() {}

void impl::printAnsiCommand(std::string command) {
    std::cout << ANSI_ESC"[" << command;
    std::cout.flush();
}

void impl::setPrompt(std::string prompt) {
    auto str = parseCharacterCodes(std::move(prompt));
    if(this->prompt == str) return;
    this->prompt = str;
    this->redrawLine();
}

void impl::redrawLine(bool lockMutex) {
    if(lockMutex){
        lock_guard<std::mutex> lock(this->mutex);
        this->redrawLine(false);
        return;
    }

    std::stringstream ss;
    ss << "\r" << prompt << std::string(&(cursorBuffer[0]), cursorBuffer.size());
    ssize_t size = ss.str().length() - 1;
    ssize_t moveBack = size - this->_cursorPosition - this->prompt.size();

    ss << ANSI_ESC"[K";
    if(moveBack > 0){
        ss << ANSI_ESC"[" + std::to_string(moveBack)+"D";
    }

    std::cout << ANSI_RESET << ss.str() << flush;
}

void impl::writeMessage(std::string message, bool noCharacterCodes) {
    lock_guard<std::mutex> lock(this->mutex);

    if(!noCharacterCodes)
        std::cout << ANSI_RESET"\r" << parseCharacterCodes(message) << ANSI_ESC"[K" << std::endl;
    else
        std::cout << ANSI_RESET"\r" << message << ANSI_ESC"[K" << std::endl;
    redrawLine(false);
}

std::string impl::getNextLine(){
    lock_guard<std::mutex> lock(this->bufferMutex);

    auto size = this->lineBuffer.size();
    if(size > 0){
        auto line = std::move(this->lineBuffer.front());
        this->lineBuffer.pop_front();
        return line;
    }
    return "";
}

size_t impl::linesAvailable() {
    lock_guard<std::mutex> lock(this->bufferMutex);
    return this->lineBuffer.size();
}

std::string impl::readLine(const std::string& promt, time_point<system_clock> timeout) {
    bool havingTimeout = timeout.time_since_epoch().count() == 0;

    std::string currentLine;
    while ((currentLine = getNextLine()).empty() && (havingTimeout && system_clock::now() < timeout)){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return currentLine;
}

std::string impl::getCursorBuffer() {
    lock_guard<std::mutex> lock(this->bufferMutex);

    return std::string(&(this->cursorBuffer[0]), cursorBuffer.size());
}

void impl::cursorPosition(std::string buffer) {
    lock_guard<std::mutex> lock(this->bufferMutex);

    int64_t oldSize = this->cursorBuffer.size();
    this->cursorBuffer.clear();
    for (char i : buffer)
        this->cursorBuffer.push_back(i);
    int64_t moveOffset = this->cursorBuffer.size() - oldSize;
    /*
    if(move < 0){
        std::cout << "\x1B[" + std::to_string(-move)+"D";
        std::cout.flush();
    } else {
        std::cout << "\x1B[" + std::to_string(move)+"C";
        std::cout.flush();
    }
    */
    this->_cursorPosition += moveOffset;
    this->redrawLine(false);
}

size_t impl::cursorPosition() {
    return this->_cursorPosition;
}

void impl::setCursorPosition(size_t index) { //TODO bounds check!
    ssize_t move = index - this->_cursorPosition ;
    if(move < 0){
        std::cout << ANSI_ESC"[" + std::to_string(-move)+"D";
        std::cout.flush();
    } else {
        std::cout << ANSI_ESC"[" + std::to_string(move)+"C";
        std::cout.flush();
    }
    this->_cursorPosition = index;
}

void impl::addTabCompleter(TabCompleter* tabCompleter) {
    lock_guard<std::mutex> lock(this->tabCompleterLock);

    tabCompleters.push_back(tabCompleter);
}

void impl::removeTabCompleter(TabCompleter* tabCompleter) {
    lock_guard<std::mutex> lock(this->tabCompleterLock);

    auto it = std::find(tabCompleters.begin(),tabCompleters.end(), tabCompleter);
    if(it != tabCompleters.end())
        tabCompleters.erase(it);
}