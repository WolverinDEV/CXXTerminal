//
// Created by wolverindev on 12.11.16.
//

#include "../include/Terminal.h"

#include <sstream>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <sstream>
#include <csignal>
#include <functional>
#include <sys/time.h>

struct termios orig_termios;
Terminal* terminalInstance = nullptr;


Terminal* Terminal::getInstance() {
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
    terminalInstance = new Terminal;
    initNonblock();
    terminalInstance->startReader();
}

void Terminal::uninstall() {
    terminalInstance->stopReader();
    delete terminalInstance;
    terminalInstance = nullptr;
    removeNonblock();
}

int Terminal::startReader() {
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

int Terminal::stopReader() {
    if(this->readerThread != nullptr){
        pthread_cancel(*(this->readerThread));
        delete(this->readerThread);
        this->readerThread = nullptr;
        return 0;
    }
    return -1;
}

int Terminal::readNextByte() {
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

std::string Terminal::parseCharacterCodes(std::string in) {
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
                    stream << ANSI_BLACK;
                    break;
                case '1':
                    stream << ANSI_BLUE;
                    break;
                case '2':
                    stream << ANSI_GREEN;
                    break;
                case '3':
                    stream << ANSI_CYAN;
                    break;
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
                    stream << ANSI_BOLD;
                    break;
                case 'r':
                    stream << ANSI_RESET;
                    break;
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

void Terminal::printCommand(std::string command) {
    std::cout << "\x1B[" << command;
    std::cout.flush();
}

void Terminal::setPromt(std::string promt) {
    this->promt = promt;
    this->redrawLine();
}

void Terminal::redrawLine(bool lockMutex) {
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

void Terminal::writeMessage(std::string message, bool noCharacterCodes) {
    pthread_mutex_lock(&mutex);
    if(!noCharacterCodes)
        std::cout << ANSI_RESET"\r" << parseCharacterCodes(message) << "\x1B[K" << std::endl;
    else
        std::cout << ANSI_RESET"\r" << message << "\x1B[K" << std::endl;
    redrawLine(false);
    pthread_mutex_unlock(&mutex);
}

void Terminal::charReaded(int character) {
    if(character == 10){
        //std::cout << "Having line character: " << std::endl;
        pthread_mutex_lock(&(this->bufferMutex));
        lineBuffer.push_back(std::string(&(cursorBuffer[0]), cursorBuffer.size()));
        pthread_mutex_unlock(&(this->bufferMutex));
        cursorBuffer.clear();
        cursorPosition = 0;
        redrawLine();
    } else if(character == 9){
        writeMessage("Tab char");
    } else if(character == 27){
        int category = readNextByte();
        int type = readNextByte();
        //printMessage("Special char. Group: "+to_string(category)+ " type: "+to_string(type));

        if(category == 91){ //Arrow keys
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

std::string Terminal::getNextLine(){
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

int Terminal::linesAvariable() {
    pthread_mutex_lock(&(this->bufferMutex));
    int size = this->lineBuffer.size();
    pthread_mutex_unlock(&(this->bufferMutex));
    return size;
}

std::string Terminal::readLine() {
    return Terminal::readLine("");
}

std::string Terminal::readLine(std::string promt) {
    return Terminal::readLine(promt, -1);
}

std::string Terminal::readLine(std::string promt, int timeout) {
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

std::string Terminal::getCursorBuffer() {
    return std::string(&(this->cursorBuffer[0]), cursorBuffer.size());
}