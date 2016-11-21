//
// Created by wolverindev on 21.11.16.
//

#include <string>
#include <cstring>
#include "../include/Terminal.h"


using namespace std;
int main(int argsSize, char** args){
    Terminal::setup();

    TabCompleter cmp = [](string line,string word,vector<string>& cmp){
        Terminal::getInstance()->writeMessage("Tab complete: "+word);
        cmp.push_back("germany");
        cmp.push_back("world");
    };
    Terminal::getInstance()->addTabCompleter(&cmp);
    while (Terminal::isActive()){
        Terminal::getInstance()->writeMessage("Having message: "+Terminal::getInstance()->readLine());
    }
    Terminal::getInstance()->writeMessage("§aExit programm");
    return 0;
}
