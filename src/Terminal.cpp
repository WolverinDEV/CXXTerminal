#include "../include/Terminal.h"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <csignal>
#include <algorithm>
#include <sys/time.h>
#include <cstring>
#include <cxxabi.h>
#include <stdlib.h>
#include <cassert>
#include <utility>

using namespace std;

/**
 * Quick terminal for instand access
 */

void writeMessage(const std::string &message){
    if(Terminal::isActive())
        Terminal::getInstance()->writeMessage(message);
    else
        (std::cout << message << std::endl).flush();
}
bool isTerminalEnabled(){
    return Terminal::isActive();
}

/**
 * Terminal class
 */
using namespace Terminal;

struct termios orig_termios;
TerminalImpl* terminalInstance = nullptr;


Terminal::TerminalImpl* Terminal::getInstance() {
    return terminalInstance;
}

void removeNonblock(){
#ifndef TERMINAL_NON_BLOCK
    tcsetattr(0, TCSANOW, &orig_termios);
    std::cout << ANSI_RESET;
    std::cout.flush();
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

void Terminal::setup() {
    assert(!terminalInstance);

    terminalInstance = new TerminalImpl();
    initNonblock();
    terminalInstance->startReader();
}

void Terminal::uninstall() {
    assert(terminalInstance);

    terminalInstance->stopReader();
    if(!terminalInstance->getPromt().empty()) cout << "\r" << flush;
    delete terminalInstance;
    terminalInstance = nullptr;
    removeNonblock();
}

bool Terminal::isActive() {
    return terminalInstance != nullptr;
}

int TerminalImpl::startReader() {
    if(this->readerThread == nullptr){
        this->running = true;
        this->readerThread = new std::thread([&](){
            while(this->running){
                auto readed = this->readNextByte();
                if(readed == -1) continue;
                this->charReaded(readed);
            }
        });
        return 1;
    }
    return -1;
}

int TerminalImpl::stopReader() {
    if(this->readerThread != nullptr){
        this->running = false;
        if(this->readerThread->joinable())
            this->readerThread->join();
        delete this->readerThread;
        this->readerThread = nullptr;
        return 0;
    }
    return -1;
}

int TerminalImpl::readNextByte() {
    int read;
    while(this->running){
        read = getchar();
        if(read >= 0) return read;
        usleep(1000);
    }
    return -1;
}

std::string TerminalImpl::parseCharacterCodes(std::string in) {
    stringstream out;
    size_t index = 0;
    size_t oldIndex = 0;
    while((index = in.find('§', oldIndex)) > 0 && index < in.size()){
        out << in.substr(oldIndex, index - oldIndex - 1);
        switch (tolower(in.substr(index + 1, 1)[0])){
            case '0':
                out << ANSI_BLACK; break;
            case '1':
                out << ANSI_BLUE; break;
            case '2':
                out << ANSI_GREEN; break;
            case '3':
                out << ANSI_CYAN; break;
            case '4':
                out << ANSI_RED; break;
            case '5':
                out << ANSI_PURPLE; break;
            case '6':
                out << ANSI_BROWN; break;
            case '7':
                out << ANSI_GRAY; break;
            case '8':
                out << ANSI_DARK_GREY; break;
            case '9':
                out << ANSI_LIGHT_BLUE; break;
            case 'a':
                out << ANSI_LIGHT_GREEN; break;
            case 'b':
                out << ANSI_LIGHT_CYAN; break;
            case 'c':
                out << ANSI_LIGHT_RED; break;
            case 'd':
                out << ANSI_LIGHT_PURPLE; break;
            case 'e':
                out << ANSI_YELLOW; break;
            case 'f':
                out << ANSI_WHITE; break;
            case 'k': break;
            case 'n':
                out << ANSI_UNDERLINE; break;
            case 'm':
                out << ANSI_UNDERLINE << ANSI_REVERSE; break;
            case 'o': break;
            case 'l':
                out << ANSI_BOLD; break;
            case 'r':
                out << ANSI_RESET; break;
            default:
                out << "§";
                index -= 1;
        }
        index += 2;
        oldIndex = index;
    }
    out << in.substr(oldIndex);
    return out.str();
}

std::string TerminalImpl::stripCharacterCodes(std::string in) {
    stringstream out;
    size_t index = 0;
    size_t oldIndex = 0;
    while((index = in.find('§', oldIndex)) > 0 && index < in.size()){
        out << in.substr(oldIndex, index - oldIndex - 1);
        switch (tolower(in.substr(index + 1, 1)[0])){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'k':
            case 'n':
            case 'm':
            case 'o':
            case 'l':
            case 'r':
                break;
            default:
                out << "§";
                index -= 1;
        }
        index += 2;
        oldIndex = index;
    }
    out << in.substr(oldIndex);
    return out.str();
}

void TerminalImpl::printCommand(std::string command) {
    std::cout << "\x1B[" << command;
    std::cout.flush();
}

void TerminalImpl::setPromt(std::string promt) {
    this->promt = parseCharacterCodes(std::move(promt));
    this->redrawLine();
}

void TerminalImpl::redrawLine(bool lockMutex) {
    try {
        if(lockMutex) pthread_mutex_lock(&mutex);
        std::stringstream ss;
        ss << "\r" << promt << std::string(&(cursorBuffer[0]), cursorBuffer.size());
        auto size = ss.str().length();
        auto asize = size - 1;
        auto moveBack = asize - this->cursorPosition - this->promt.size();

        ss << "\x1B[K";
        if(moveBack > 0){
            ss << "\x1B[" + std::to_string(moveBack)+"D";
        }

        std::cout << ANSI_RESET << ss.str();
        std::cout.flush();

        if(lockMutex) pthread_mutex_unlock(&mutex);
    } catch(const abi::__forced_unwind&){
        if(lockMutex) pthread_mutex_unlock(&mutex);
        throw;
    } catch (...){
        if(lockMutex) pthread_mutex_unlock(&mutex);
        throw;
    }
}

void TerminalImpl::writeMessage(std::string message, bool noCharacterCodes) {
    int oldCancelState;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
    try {
        pthread_mutex_lock(&mutex);
        if(!noCharacterCodes)
            std::cout << ANSI_RESET"\r" << parseCharacterCodes(message) << "\x1B[K" << std::endl;
        else
            std::cout << ANSI_RESET"\r" << message << "\x1B[K" << std::endl;
        redrawLine(false);
        pthread_mutex_unlock(&mutex);
    } catch (...){
        std::cout << "writeMessage() error" << std::endl;
        pthread_mutex_unlock(&mutex);
        if(oldCancelState != PTHREAD_CANCEL_DISABLE) pthread_setcancelstate(oldCancelState, nullptr);
        throw;
    }
    if(oldCancelState != PTHREAD_CANCEL_DISABLE) pthread_setcancelstate(oldCancelState, nullptr);
}

template <typename T>
using JoinFunction = std::function<std::string(T)>;

template <typename T>
std::string join(std::vector<T> elm,JoinFunction<T> toStringFunc){
    std::stringstream out;
    for(auto it = elm.begin();it != elm.end();it++)
        out << ", " << toStringFunc(*it);
    return out.str().length() > 2 ? out.str().substr(2) : out.str();
}

JoinFunction<std::string> joinStrings = [](std::string elm){
    return elm;
};

void TerminalImpl::charReaded(int character) {
    if(character == 10){
        pthread_mutex_lock(&(this->bufferMutex));
        std::string line = std::string(&(cursorBuffer[0]), cursorBuffer.size());
        lineBuffer.push_back(line);
        commandHistory.push_back(line);
        while (commandHistory.size() > HISTORY_LINE_BUFFER_SIZE){
            commandHistory.erase(commandHistory.begin());
        }
        this->historyIndex = 0;

        pthread_mutex_unlock(&(this->bufferMutex));
        cursorBuffer.clear();
        cursorPosition = 0;
        redrawLine();
    } else if(character == 9){
        if(newInputTyped){
            int inc = 0;
            if(this->cursorPosition == this->cursorBuffer.size()){
                this->currentTabComplete.clear();
                std::string buffer = getCursorBuffer();
                int lastIndex = buffer.find_last_of(' ');
                std::string lastBuffer = buffer.substr(lastIndex == -1 ? 0 : lastIndex + 1);
                for(std::vector<TabCompleter*>::iterator it = this->tabCompleters.begin(); it != tabCompleters.end();it++)
                    it.operator*()->operator()(buffer, lastBuffer,this->currentTabComplete);

                std::string suggested = join(this->currentTabComplete, joinStrings);
                if(suggested.size() != 0)
                    writeMessage(suggested);
                if(lastBuffer.find_first_not_of(' ') != -1 && std::find(this->currentTabComplete.begin(), this->currentTabComplete.end(), lastBuffer.substr(1)) == this->currentTabComplete.end())
                    this->currentTabComplete.push_back(lastBuffer);
                else
                    inc++;

                if(this->currentTabComplete.size() > 0){
                    setCursorBuffer(buffer.substr(0, lastIndex == -1 ? 0 : lastIndex)  + (lastIndex == -1 ? "" : " ") + this->currentTabComplete[0]);
                    this->tabCompleteIndex = 0;
                } else
                    this->tabCompleteIndex = -1;
                newInputTyped = this->currentTabComplete.size() + inc < 3; //DOnt need a circle
            }
            else {
                int next;
                if(this->cursorBuffer.size() > this->cursorPosition + 1)
                    next = getCursorBuffer().find_first_of(' ', this->cursorPosition + 1);
                else
                    next = -1;
                setCursorPosition(next == -1 ? this->cursorBuffer.size() : next);
                redrawLine(false);
            }
        } else {
            if(this->tabCompleteIndex < 0)
                return;
            this->tabCompleteIndex++;
            if(this->currentTabComplete.size() <= this->tabCompleteIndex)
                this->tabCompleteIndex = 0;
            std::string fullBuffer = this->getCursorBuffer();
            int lastIndex = fullBuffer.find_last_of(' ');
            setCursorBuffer(fullBuffer.substr(0, lastIndex == -1 ? 0 : lastIndex) + (lastIndex == -1 ? "" : " ") + this->currentTabComplete[this->tabCompleteIndex]);
        }

    } else if(character == 27){
        int category = readNextByte();
        int type = readNextByte();
        //writeMessage("Special char. Group: "+std::to_string(category)+ " type: "+std::to_string(type));

        if(category == 91){ //Arrow keys
            newInputTyped = true;
            if(type == 68){
                if(cursorPosition > 0){
                    this->printCommand("1D");
                    this->cursorPosition--;
                } else return;
            } else if(type == 67){
                if(cursorPosition < this->cursorBuffer.size()) {
                    this->printCommand("1C");
                    this->cursorPosition++;
                } else return;
            } else if(type == 65){ //Arow up
                if(this->historyIndex < this->commandHistory.size()){
                    this->historyIndex++;
                    this->setCursorBuffer(this->commandHistory[this->commandHistory.size()-historyIndex]); //Invert
                }
            } else if(type == 66){ //Arow down
                if(this->historyIndex > 0){
                    this->historyIndex--;
                    if(this->historyIndex == 0)
                        this->setCursorBuffer("");
                    else
                        this->setCursorBuffer(this->commandHistory[this->commandHistory.size()-historyIndex]); //Invert
                }
            }
            //printMessage("New cursor position: "+to_string(cursorPosition));
        }
    } else if(character == 127){
        if(cursorPosition > 0){
            cursorPosition--;
            cursorBuffer.erase(cursorBuffer.begin()+cursorPosition);
            this->newInputTyped = true;
            redrawLine();
        } else return;
    } else {
        //printMessage("having character: "+to_string(character)+" curso position: "+to_string(cursorPosition)+" buffersize: "+to_string(cursorBuffer.size()));
        if(isprint(character)){
            newInputTyped = true;
            if(cursorPosition < cursorBuffer.size()){
                cursorBuffer.insert(cursorBuffer.begin()+cursorPosition, character);
            }
            else
                cursorBuffer.push_back((char) character);
            cursorPosition++;
            redrawLine();
        }
    }
}


inline uint64_t getCurrentTimeMillis(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    uint64_t ms = tp.tv_sec * 1000*1000 + tp.tv_usec;
    return ms / 1000;
}

std::string TerminalImpl::getNextLine(){
    pthread_mutex_lock(&(this->bufferMutex));
    auto size = this->lineBuffer.size();
    if(size > 0){
        auto line = *this->lineBuffer.begin();
        this->lineBuffer.erase(this->lineBuffer.begin());
        pthread_mutex_unlock(&(this->bufferMutex));
        return line;
    }
    pthread_mutex_unlock(&(this->bufferMutex));
    return "";
}

int TerminalImpl::linesAvariable() {
    pthread_mutex_lock(&(this->bufferMutex));
    int size = this->lineBuffer.size();
    pthread_mutex_unlock(&(this->bufferMutex));
    return size;
}

std::string TerminalImpl::readLine(const std::string& promt, int timeout) {
    uint64_t current = getCurrentTimeMillis();
    if(timeout >= 0){
        timespec spec = {timeout / 1000, (timeout % 1000) * 1000};
        int state = pthread_mutex_timedlock(&(this->readlineMutex), &spec);
        if(state != 0){
            return "";
        }
    } else
        pthread_mutex_lock(&(this->readlineMutex));
    this->setPromt(promt);
    std::string currentLine;
    while ((currentLine = getNextLine()).empty() && (timeout < 0 || current + timeout > getCurrentTimeMillis())){
        usleep(1000);
    }
    pthread_mutex_unlock(&(this->readlineMutex));
    return currentLine;
}

std::string TerminalImpl::getCursorBuffer() {
    return std::string(&(this->cursorBuffer[0]), cursorBuffer.size());
}

void TerminalImpl::setCursorBuffer(std::string buffer) {
    pthread_mutex_lock(&(this->bufferMutex));
    int oldSize = this->cursorBuffer.size();
    this->cursorBuffer.clear();
    for(int i = 0;i<buffer.length();i++)
        this->cursorBuffer.push_back(buffer[i]);
    int move = this->cursorBuffer.size() - oldSize;
    /*
    if(move < 0){
        std::cout << "\x1B[" + std::to_string(-move)+"D";
        std::cout.flush();
    } else {
        std::cout << "\x1B[" + std::to_string(move)+"C";
        std::cout.flush();
    }
    */
    this->cursorPosition += move;
    this->redrawLine(false);
    pthread_mutex_unlock(&(this->bufferMutex));
}

int TerminalImpl::getCursorPosition() {
    return this->cursorPosition;
}

void TerminalImpl::setCursorPosition(int index) { //TODO bounds check!
    int move = index - this->cursorPosition ;
    if(move < 0){
        std::cout << "\x1B[" + std::to_string(-move)+"D";
        std::cout.flush();
    } else {
        std::cout << "\x1B[" + std::to_string(move)+"C";
        std::cout.flush();
    }
    this->cursorPosition = index;
}

void TerminalImpl::addTabCompleter(TabCompleter* tabCompleter) {
    tabCompleters.push_back(tabCompleter);
}

void TerminalImpl::removeTabCompleter(TabCompleter* tabCompleter) {
    auto it = std::find(tabCompleters.begin(),tabCompleters.end(), tabCompleter);
    for(;it != tabCompleters.end();it++)
        tabCompleters.erase(it);

}