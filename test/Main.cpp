//
// Created by wolverindev on 21.11.16.
//

#include <string>
#include <algorithm>
#include <typeinfo>
#include "../include/QuickTerminal.h"


using namespace std;
int main(int argsSize, char** args){
    Terminal::setup();

    writeMessage(string(typeid(Terminal).name()));
    if(1)
        return 0;
    TabCompleter cmp = [](string line,string word,vector<string>& cmp){
        Terminal::getInstance()->writeMessage("Tab complete: "+word);
        cmp.push_back("germany");
        cmp.push_back("world");
    };
    Terminal::getInstance()->addTabCompleter(&cmp);
    while (Terminal::isActive()){
        string message = Terminal::getInstance()->readLine();
        string lmessage = string(message);
        std::transform(lmessage.begin(), lmessage.end(), lmessage.begin(), ::tolower);

        Terminal::getInstance()->writeMessage("Having message: "+message);
        if(lmessage.compare("exit") == 0 || lmessage.compare("q") == 0 || lmessage.compare("quit") == 0){
            Terminal::getInstance()->writeMessage("§aExit program");
            Terminal::uninstall();
        }
    }
    return 0;
}
