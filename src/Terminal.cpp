//
// Created by wolverindev on 12.11.16.
//

#include "../include/Terminal.h"
#include "../include/QuickTerminal.h"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <csignal>
#include <algorithm>
#include <sys/time.h>


/**
 * Quick terminal for instand access
 */

void writeMessage(std::string message){
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
    tcsetattr(0, TCSANOW, &orig_termios);
    std::cout << ANSI_RESET"\r";
    std::cout.flush();
}

void initNonblock(){
    struct termios new_termios;

    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    atexit(removeNonblock);
    //cfmakeraw(&new_termios);

    new_termios.c_lflag &= ~ICANON;
    new_termios.c_lflag &= ~ECHO;
    new_termios.c_lflag &= ~ISIG;
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &new_termios);
}

void* terminalReaderThread(void* args){
    ReaderThreadArgs* handle = (ReaderThreadArgs*) args;

    int current;
    while (std::cin && *(handle->handlePtr) != nullptr) {
        current = handle->readFunction();
        if(current > 0){
            handle->readedFunction(current);
            continue;
        }
        usleep(1000);
    }
}

void Terminal::setup() {
    terminalInstance = new TerminalImpl;
    initNonblock();
    terminalInstance->startReader();
}

void Terminal::uninstall() {
    terminalInstance->stopReader();
    delete terminalInstance;
    terminalInstance = nullptr;
    removeNonblock();
}

bool Terminal::isActive() {
    return terminalInstance != nullptr;
}

int TerminalImpl::startReader() {
    if(this->readerThread == nullptr){
        this->readerThread = new pthread_t;

        ReaderThreadArgs* ptrArgs = new ReaderThreadArgs;
        ReaderThreadArgs& initArgs = *ptrArgs;
        initArgs.handlePtr = &terminalInstance;
        initArgs.readFunction = [&](){
            return this->readNextByte();
        };
        initArgs.readedFunction = [&](int character){
            this->charReaded(character);
        };

        return pthread_create(readerThread, nullptr, terminalReaderThread, ptrArgs);
    }
    return -1;
}

int TerminalImpl::stopReader() {
    if(this->readerThread != nullptr){
        pthread_cancel(*(this->readerThread));
        delete(this->readerThread);
        this->readerThread = nullptr;
        return 0;
    }
    return -1;
}

int TerminalImpl::readNextByte() {
    int readed;
    while ((readed = getchar()) < 1){
        usleep(1000);
    }
    return readed;
}

inline void split(const std::string &s, std::string delim, std::vector<std::string> &elems) {
    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
        elems.push_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);
    }
    elems.push_back(s.substr(start, end - start));
}

std::string TerminalImpl::parseCharacterCodes(std::string in) {
    std::stringstream stream;
    std::vector<std::string> parts;
    split(in, "§", parts);

    bool first = true;
    for(std::vector<std::string>::iterator it = parts.begin(); it != parts.end();it++) {
        int index = 0;
        if(!first){
            index = 1;
            std::string first = (*it).substr(0,1);
            switch (tolower(first[0])){
                case '0':
                    stream << ANSI_BLACK; break;
                case '1':
                    stream << ANSI_BLUE; break;
                case '2':
                    stream << ANSI_GREEN; break;
                case '3':
                    stream << ANSI_CYAN; break;
                case '4':
                    stream << ANSI_RED; break;
                case '5':
                    stream << ANSI_PURPLE; break;
                case '6':
                    stream << ANSI_BROWN; break;
                case '7':
                    stream << ANSI_GRAY; break;
                case '8':
                    stream << ANSI_DARK_GREY; break;
                case '9':
                    stream << ANSI_LIGHT_BLUE; break;
                case 'a':
                    stream << ANSI_LIGHT_GREEN; break;
                case 'b':
                    stream << ANSI_LIGHT_CYAN; break;
                case 'c':
                    stream << ANSI_LIGHT_RED; break;
                case 'd':
                    stream << ANSI_LIGHT_PURPLE; break;
                case 'e':
                    stream << ANSI_YELLOW; break;
                case 'f':
                    stream << ANSI_WHITE; break;
                case 'n':
                    stream << ANSI_UNDERLINE; break;
                case 'm':
                    stream << ANSI_UNDERLINE << ANSI_REVERSE; break;
                case 'o':
                    break;
                case 'l':
                    stream << ANSI_BOLD; break;
                case 'r':
                    stream << ANSI_RESET; break;
                default:
                    stream << "§";
                    index = 0;
            }
        }
        else
            first = false;
        stream << (*it).substr(index);
    }
    stream << ANSI_RESET;
    return stream.str();
}

void TerminalImpl::printCommand(std::string command) {
    std::cout << "\x1B[" << command;
    std::cout.flush();
}

void TerminalImpl::setPromt(std::string promt) {
    this->promt = promt;
    this->redrawLine();
}

void TerminalImpl::redrawLine(bool lockMutex) {
    if(lockMutex)
        pthread_mutex_lock(&(this->mutex));
    std::stringstream ss;
    ss << "\r" << promt << std::string(&(cursorBuffer[0]), cursorBuffer.size());
    int size = ss.str().length();
    int asize = size - 1;

    int moveBack = asize - this->cursorPosition - this->promt.size();

    ss << "\x1B[K";
    if(moveBack > 0){
        ss << "\x1B[" + std::to_string(moveBack)+"D";
    }

    std::cout << ANSI_RESET << ss.str();
    std::cout.flush();

    if(lockMutex)
        pthread_mutex_unlock(&(this->mutex));
}

void TerminalImpl::writeMessage(std::string message, bool noCharacterCodes) {
    pthread_mutex_lock(&mutex);
    if(!noCharacterCodes)
        std::cout << ANSI_RESET"\r" << parseCharacterCodes(message) << "\x1B[K" << std::endl;
    else
        std::cout << ANSI_RESET"\r" << message << "\x1B[K" << std::endl;
    redrawLine(false);
    pthread_mutex_unlock(&mutex);
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
    } else if(character == 11){
        //printMessage("Having 11 char!");
        int readed = readNextByte();
        if(readed == 3){
            exit(1);
            //raise(SIGABRT);
        } else;
        //printMessage("Readed: "+to_string(readed));
    } else if(character == 127){
        if(cursorPosition > 0){
            cursorPosition--;
            cursorBuffer.erase(cursorBuffer.begin()+cursorPosition);
            //cursorBuffer.erase(cursorBuffer.begin()+(cursorPosition), cursorBuffer.begin()+(cursorPosition+1));
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
    int size = this->lineBuffer.size();
    if(size > 0){
        auto line = this->lineBuffer.begin();
        this->lineBuffer.erase(line);
        pthread_mutex_unlock(&(this->bufferMutex));
        return std::string(line.operator*().c_str());
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

std::string TerminalImpl::readLine() {
    return TerminalImpl::readLine("");
}

std::string TerminalImpl::readLine(std::string promt) {
    return TerminalImpl::readLine(promt, -1);
}

std::string TerminalImpl::readLine(std::string promt, int timeout) {
    uint64_t current = getCurrentTimeMillis();
    if(timeout > 0){
        timespec spec = {timeout / 1000, (timeout % 1000) * 1000};
        int state = pthread_mutex_timedlock(&(this->readlineMutex), &spec);
        if(state != 0){
            return "";
        }
    } else
        pthread_mutex_lock(&(this->readlineMutex));
    this->setPromt(promt);
    std::string currentLine = "";
    while ((currentLine = getNextLine()) == "" && (timeout <= 0 || current + timeout > getCurrentTimeMillis())){
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