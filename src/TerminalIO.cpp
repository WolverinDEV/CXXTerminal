#include <algorithm>
#include <sstream>
#include <cassert>
#include <iostream>
#include <cstring>
#include <csignal>
#include "../include/Terminal.h"
#ifdef USE_LIBEVENT
    #include <event2/thread.h>
#endif
#ifdef WIN32
    #define STDIN_FILENO (0)
#else
	#include <zconf.h>
#endif

using namespace std;
using namespace terminal;

template <typename T>
using JoinFunction = std::function<std::string(T)>;

template <typename T>
std::string join(std::vector<T> elm,const JoinFunction<T>& toStringFunc){
    stringstream out;
    for(auto it = elm.begin();it != elm.end();it++)
        out << ", " << toStringFunc(*it);
    return out.str().length() > 2 ? out.str().substr(2) : out.str();
}


int impl::startReader() {
#ifdef EVTHREAD_USE_PTHREADS_IMPLEMENTED
    if(evthread_use_pthreads() != 0)
        cerr << "failed to execute evthread_use_pthreads()" << endl;
#endif

    assert(!this->readerThread);
    this->running = true;

#ifndef USE_LIBEVENT
    this->readerThread = new std::thread([&](){
        while(this->running){
            auto readed = this->readNextByte();
            if(readed == -1) continue;
            this->rdbuf += (char) readed;
            while(this->handleRead());
        }
    });
    return 0;
#else
    this->eventLoop = event_base_new();
    this->readEvent = event_new(this->eventLoop, STDIN_FILENO, EV_READ | EV_PERSIST, [](evutil_socket_t a, short b, void* c){ ((impl*) c)->handleInput(a, b, c); }, this);
    event_add(readEvent, nullptr);

    this->readerThread = new std::thread([&](){
        while(this->running) {
            event_base_loop(this->eventLoop, 0);
            std::this_thread::sleep_for(chrono::milliseconds(500));
        }
    });
    return 0;
#endif
    return -1;
}

int impl::stopReader() {
    assert(this->readerThread);
    assert(this->readerThread->get_id() != this_thread::get_id());

    this->running = false;

#ifndef USE_LIBEVENT
    if(this->readerThread->joinable())
        this->readerThread->join();
    delete this->readerThread;
    this->readerThread = nullptr;
    return 0;
#else
    if(this->readEvent) {
        event_del_block(this->readEvent);
        event_free(this->readEvent);
        this->readEvent = nullptr;
    }

    if(this->eventLoop) {
        auto result = event_base_loopexit(this->eventLoop, nullptr);
        if(result != 0) cerr << "Failed to break event loop!" << endl;
    }

    if(this->readerThread && this->readerThread->joinable())
        this->readerThread->join();

    if(this->eventLoop) {
        event_base_free(this->eventLoop);
        this->eventLoop = nullptr;
    }

    delete this->readerThread;
    this->readerThread = nullptr;

#endif
    return -1;
}

#ifndef USE_LIBEVENT
int impl::readNextByte() {
    int read;
    while(this->running){
        read = getchar();
        if(read >= 0) return read;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return -1;
}
#else
#define RD_BUFFER_SIZE 1024
void impl::handleInput(int fd, short, void *) {
    char buffer[RD_BUFFER_SIZE];
#ifndef WIN32
    auto read = ::read(fd, buffer, RD_BUFFER_SIZE);
#else
    auto read = 0;
#endif
    if(read < 0){
        stringstream ss;
        ss << ANSI_RED << "Invalid terminal read. errno: " << errno << " msg: " << strerror(errno) << endl;
        writeMessage(ss.str());
    } else if(read > 0){
        this->rdbuf += string(buffer, read);
    }
    while(this->handleRead());
}
#endif

bool impl::handleRead() {
    lock_guard<::mutex> rdBufLock(this->rdbufLock);
    if(this->rdbuf.empty()) return false;

    int character = this->rdbuf[0];

    if(character == 10){
        this->rdbuf = this->rdbuf.substr(1);
        lock_guard<std::mutex> lock(this->bufferMutex);

        std::string line = std::string(&(cursorBuffer[0]), cursorBuffer.size());
        lineBuffer.push_back(line);
        commandHistory.push_back(line);
        while (commandHistory.size() > this->maxHistorySize)
            commandHistory.erase(commandHistory.begin());
        this->historyIndex = 0;

        cursorBuffer.clear();
        _cursorPosition = 0;
        redrawLine();
    } else if(character == 9){
        this->rdbuf = this->rdbuf.substr(1);
        if(newInputTyped){
            int inc = 0;
            if(this->_cursorPosition == this->cursorBuffer.size()){
                this->currentTabComplete.clear();
                string buffer = getCursorBuffer();
                size_t lastIndex = buffer.find_last_of(' ');
                string lastBuffer = buffer.substr(lastIndex == string::npos ? 0 : lastIndex + 1);

                for(const auto& elm : this->tabCompleters)
                    if(elm)
                        (*elm)(buffer, lastBuffer, this->currentTabComplete);

                std::string suggested = join<string>(this->currentTabComplete, [](string e) { return e; });
                if(!suggested.empty())
                    writeMessage(suggested);
                if(lastBuffer.find_first_not_of(' ') != -1 && find(this->currentTabComplete.begin(), this->currentTabComplete.end(), lastBuffer.substr(1)) == this->currentTabComplete.end())
                    this->currentTabComplete.push_back(lastBuffer);
                else
                    inc++;

                if(!this->currentTabComplete.empty()){
                    cursorPosition(buffer.substr(0, lastIndex == -1 ? 0 : lastIndex) + (lastIndex == -1 ? "" : " ") +
                                   this->currentTabComplete[0]);
                    this->tabCompleteIndex = 0;
                } else
                    this->tabCompleteIndex = -1;
                newInputTyped = this->currentTabComplete.size() + inc < 3; //DOnt need a circle
            }
            else {
                size_t next;
                if(this->cursorBuffer.size() > this->_cursorPosition + 1)
                    next = getCursorBuffer().find_first_of(' ', this->_cursorPosition + 1);
                else
                    next = string::npos;
                setCursorPosition(next == string::npos ? this->cursorBuffer.size() : next);
                redrawLine(false);
            }
        } else {
            if(this->tabCompleteIndex < 0)
                return true;
            this->tabCompleteIndex++;
            if(this->currentTabComplete.size() <= this->tabCompleteIndex)
                this->tabCompleteIndex = 0;
            std::string fullBuffer = this->getCursorBuffer();
            size_t lastIndex = fullBuffer.find_last_of(' ');
            cursorPosition(fullBuffer.substr(0, lastIndex == string::npos ? 0 : lastIndex) +
                           (lastIndex == string::npos ? "" : " ") + this->currentTabComplete[this->tabCompleteIndex]);
        }
        return true;
    } else if(character == 27){
        if(rdbuf.size() < 3) return false;
        int category = rdbuf[1];
        int type = rdbuf[2];
        this->rdbuf = this->rdbuf.substr(3);
        //writeMessage("Special char. Group: "+std::to_string(category)+ " type: "+std::to_string(type));

        if(category == 91){ //Arrow keys
            newInputTyped = true;
            if(type == 68){
                if(_cursorPosition > 0){
                    //this->_move_cursor(-1, 0, true);
                    this->printAnsiCommand("1D");
                    this->_cursorPosition--;
                } else return true;
            } else if(type == 67){
                if(_cursorPosition < this->cursorBuffer.size()) {
                    //this->_move_cursor(1, 0, true);
                    this->printAnsiCommand("1C");
                    this->_cursorPosition++;
                } else return true;
            } else if(type == 65){ //Arow up
                if(this->historyIndex < this->commandHistory.size()){
                    this->historyIndex++;
                    this->cursorPosition(this->commandHistory[this->commandHistory.size() - historyIndex]); //Invert
                }
            } else if(type == 66){ //Arow down
                if(this->historyIndex > 0){
                    this->historyIndex--;
                    if(this->historyIndex == 0)
                        this->cursorPosition("");
                    else
                        this->cursorPosition(this->commandHistory[this->commandHistory.size() - historyIndex]); //Invert
                }
            }
            //printMessage("New cursor position: "+to_string(cursorPosition));
        }
    } else if(character == 127){
        this->rdbuf = this->rdbuf.substr(1);

        if(_cursorPosition > 0) {
            _cursorPosition--;
            cursorBuffer.erase(cursorBuffer.begin() + _cursorPosition);
            this->newInputTyped = true;
            redrawLine();
        }
    } else {
        this->rdbuf = this->rdbuf.substr(1);

        //printMessage("having character: "+to_string(character)+" curso position: "+to_string(cursorPosition)+" buffersize: "+to_string(cursorBuffer.size()));
        if(isprint(character)) {
            newInputTyped = true;
            if(_cursorPosition < cursorBuffer.size()){
                cursorBuffer.insert(cursorBuffer.begin() + _cursorPosition, (char) character);
            } else
                cursorBuffer.push_back((char) character);
            _cursorPosition++;
            redrawLine();
        }
    }
    return true;
}