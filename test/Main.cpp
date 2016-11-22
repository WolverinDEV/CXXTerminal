//
// Created by wolverindev on 21.11.16.
//

#include <string>
#include <algorithm>
#include <typeinfo>
#include <iostream>
#include <unistd.h>
#include "../include/QuickTerminal.h"
#include "../include/AdvancedTabCompleter.h"

using namespace std;

void handleLine(std::string& message){
    string lmessage = string(message);
    std::transform(lmessage.begin(), lmessage.end(), lmessage.begin(), ::tolower);

    Terminal::getInstance()->writeMessage("Having message: "+message);
    if(lmessage.compare("exit") == 0 || lmessage.compare("q") == 0 || lmessage.compare("quit") == 0){
        Terminal::getInstance()->writeMessage("§aExit program");
        Terminal::uninstall();
    }
}

int main(int argsSize, char** args){
    Terminal::setup();

    Terminal::AvTabCompleter tc;
    tc.setMatchOnStart(true);

    tc.addParameter(0, "test");
    tc.addParameter(0, "world");
    tc.addParameter(0, "worl2");

    tc.addParameter(1, "hey");
    tc.addParameter(1, "hello");

    Terminal::getInstance()->addTabCompleter(&(tc.getBasedCompleter()));

    int time = 0;
    while (Terminal::isActive()){
        if(Terminal::getInstance()->linesAvariable() > 0){
            time = 0;
            string message = Terminal::getInstance()->readLine();
            handleLine(message);
            continue;
        }

        std::string stime = to_string(time);
        Terminal::getInstance()->setPromt("["+string(time > 10*1000 ? time > 30*1000 ? ANSI_RED : ANSI_BROWN : ANSI_GREEN)+stime+ANSI_RESET"] > ");
        usleep(1000);
        time++;

    }
    return 0;
}
