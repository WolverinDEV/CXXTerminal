//
// Created by wolverindev on 21.11.16.
//

#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include "../include/QuickTerminal.h"
#include "../include/AdvancedTabCompleter.h"
#include "../include/TerminalGrapth.h"
#include "../include/ArgumentParser.h"

using namespace std;

string getDate(){
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
    std::string str(buffer);
    return str;
}

void handleLine(std::string& message){
    string lmessage = string(message);
    std::transform(lmessage.begin(), lmessage.end(), lmessage.begin(), ::tolower);

    Terminal::getInstance()->writeMessage("Having message: "+message);
    if(lmessage == "exit" || lmessage == "q" || lmessage == "quit"){
        Terminal::getInstance()->writeMessage("§aExit program");
        Terminal::uninstall();
    }
}

using namespace Terminal;
using namespace std::chrono;

int main(int argsSize, char** args){

    Terminal::setup();

    /*
    ArgumentParser parser;
    int number;
    parser.addArgument(ArgumentTypes::IntegerArgument("n", "number", ArgumentTypes::IntegerArgument::VALIDATOR_POSITIVE_OR_INF, &number, "A random number"));
    const char* message[] = {"-n", "\"1\""};
    cout << "X -> " << parser.parseArguments(2, (char**) message) << endl;
    cout << "Y -> " << number << endl;
    if(true) return 0;
     */

    AvTabCompleter cmp;
    cmp.parameter("hello")->parameter("hell!");
    cmp.parameter("hello")->parameter("World");
    cmp.parameter("world")->parameter("World");
    cmp.wildcard()->parameter("xxxx");
    cmp.unregisterWildcard();
    cmp.unregister("hello");
    Terminal::getInstance()->addTabCompleter(cmp.getBasedCompleter());

    Terminal::getInstance()->writeMessage("§aHello §bworld § X §a");

    Terminal::Graphics::Diagram::Graph table;
    table.dchar._char = '#';
    table.gussUnknownValue = true;

    table.addValue(Terminal::Graphics::Diagram::Point{(double) 0,(double) 0});
    table.addValue(Terminal::Graphics::Diagram::Point{(double) 5,(double) 1});
    table.addValue(Terminal::Graphics::Diagram::Point{(double) 10,(double) 10});

    Terminal::Graphics::Diagram::Point* out = new Terminal::Graphics::Diagram::Point;
    table.getValue(7, out);
    //writeMessage("Y at 7: "+to_string(out->y));
    Terminal::Graphics::Diagram::CoordinateSystem grapth;
    grapth.tables.push_back(table);

    grapth.startY = 0;
    grapth.endY = 12;

    grapth.startX = 0;
    grapth.endX = 12;
    grapth.stepX = 1;
    grapth.xAxisName = CString("§cHello :D");

    vector<string> vec = grapth.buildLine(120, 15, 4);
    for(auto it = vec.begin(); it != vec.end();it++)
        writeMessage(*it);
    delete out;

    int time = 0;

    auto last = system_clock::now();
    while (Terminal::isActive()){
        if(Terminal::getInstance()->linesAvariable() > 0){
            time = 0;
            string message = Terminal::getInstance()->readLine();
            handleLine(message);
            continue;
        }

        std::string stime = to_string(time);

        Terminal::getInstance()->setPromt("["+string(time > 10*1000 ? time > 30*1000 ? ANSI_RED : ANSI_BROWN : ANSI_GREEN)+stime+ANSI_RESET"] > ");
        //Terminal::getInstance()->setPromt("["+getDate()+"] > ");
        usleep(1000);
        time++;

        if(system_clock::now() - last > seconds(1)){
            last = system_clock::now();
            writeMessage("Hello world");
        }
    }
    return 0;
}
