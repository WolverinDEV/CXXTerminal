#include "../include/Terminal.h"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <csignal>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <fcntl.h>

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

struct termios orig_termios;
Impl* terminalInstance = nullptr;


terminal::Impl* terminal::instance() {
    return terminalInstance;
}

void removeNonblock(){
#ifndef TERMINAL_NON_BLOCK
    tcsetattr(0, TCSANOW, &orig_termios);
    std::cout << ANSI_RESET << flush;
#endif
}

void initNonblock(){
#ifndef TERMINAL_NON_BLOCK
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
}

void terminal::install() {
    assert(!terminalInstance);

    terminalInstance = new Impl();
    initNonblock();
    terminalInstance->startReader();
}

void terminal::uninstall() {
    assert(terminalInstance);

    terminalInstance->stopReader();
    if(!terminalInstance->getPromt().empty()) cout << "\r" << flush;
    delete terminalInstance;
    terminalInstance = nullptr;
    removeNonblock();
}

bool terminal::active() {
    return terminalInstance != nullptr;
}

void Impl::printCommand(std::string command) {
    std::cout << "\x1B[" << command;
    std::cout.flush();
}

void Impl::setPromt(std::string promt) {
    auto str = parseCharacterCodes(std::move(promt));
    if(this->promt == str) return;
    this->promt = str;
    this->redrawLine();
}

void Impl::redrawLine(bool lockMutex) {
    if(lockMutex){
        lock_guard<std::mutex> lock(this->mutex);
        this->redrawLine(false);
        return;
    }

    std::stringstream ss;
    ss << "\r" << promt << std::string(&(cursorBuffer[0]), cursorBuffer.size());
    ssize_t size = ss.str().length() - 1;
    ssize_t moveBack = size - this->cursorPosition - this->promt.size();

    ss << "\x1B[K";
    if(moveBack > 0){
        ss << "\x1B[" + std::to_string(moveBack)+"D";
    }

    std::cout << ANSI_RESET << ss.str() << flush;
}

void Impl::writeMessage(std::string message, bool noCharacterCodes) {
    lock_guard<std::mutex> lock(this->mutex);

    if(!noCharacterCodes)
        std::cout << ANSI_RESET"\r" << parseCharacterCodes(message) << "\x1B[K" << std::endl;
    else
        std::cout << ANSI_RESET"\r" << message << "\x1B[K" << std::endl;
    redrawLine(false);
}

std::string Impl::getNextLine(){
    lock_guard<std::mutex> lock(this->bufferMutex);

    auto size = this->lineBuffer.size();
    if(size > 0){
        auto line = std::move(this->lineBuffer.front());
        this->lineBuffer.pop_front();
        return line;
    }
    return "";
}

size_t Impl::linesAvailable() {
    lock_guard<std::mutex> lock(this->bufferMutex);
    return this->lineBuffer.size();
}

std::string Impl::readLine(const std::string& promt, time_point<system_clock> timeout) {
    bool havingTimeout = timeout.time_since_epoch().count() == 0;

    std::string currentLine;
    while ((currentLine = getNextLine()).empty() && (havingTimeout && system_clock::now() < timeout)){
        usleep(1000);
    }
    return currentLine;
}

std::string Impl::getCursorBuffer() {
    lock_guard<std::mutex> lock(this->bufferMutex);

    return std::string(&(this->cursorBuffer[0]), cursorBuffer.size());
}

void Impl::setCursorBuffer(std::string buffer) {
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
    this->cursorPosition += moveOffset;
    this->redrawLine(false);
}

size_t Impl::getCursorPosition() {
    return this->cursorPosition;
}

void Impl::setCursorPosition(size_t index) { //TODO bounds check!
    ssize_t move = index - this->cursorPosition ;
    if(move < 0){
        std::cout << "\x1B[" + std::to_string(-move)+"D";
        std::cout.flush();
    } else {
        std::cout << "\x1B[" + std::to_string(move)+"C";
        std::cout.flush();
    }
    this->cursorPosition = index;
}

void Impl::addTabCompleter(TabCompleter* tabCompleter) {
    lock_guard<std::mutex> lock(this->tabCompleterLock);

    tabCompleters.push_back(tabCompleter);
}

void Impl::removeTabCompleter(TabCompleter* tabCompleter) {
    lock_guard<std::mutex> lock(this->tabCompleterLock);

    auto it = std::find(tabCompleters.begin(),tabCompleters.end(), tabCompleter);
    if(it != tabCompleters.end())
        tabCompleters.erase(it);
}