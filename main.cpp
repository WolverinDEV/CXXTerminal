#include <iostream>
#include "include/Terminal.h"
#include <unistd.h>

using namespace std;
int main() {
    Terminal::setup();
    Terminal::getInstance()->writeMessage("§aHello world");
    int count = 20 * 1000;
    int mscount = 0;
    while (1){
        string ms = to_string(count%1000);
        ms = ("000"+ms).substr(ms.size(), 3 + ms.size());
        string sec = to_string(count/1000);
        sec = ("000"+sec).substr(sec.size(), 3 + sec.size());

        string prefix = ANSI_RED"("+sec+"."+ms+") "ANSI_GRAY"> "ANSI_BLUE ANSI_BOLD;
        if(Terminal::getInstance()->linesAvariable() > 0){
            string line = Terminal::getInstance()->readLine(prefix, 1);
            Terminal::getInstance()->writeMessage("Having line: "+line);
            count = 20 * 1000;
            continue;
        }
        Terminal::getInstance()->setPromt(prefix);
        count--;
        if(count < 0){
            Terminal::getInstance()->writeMessage("§cTimeout!");
            return 0;
        }
        usleep(1000);
    }
    return 0;
}