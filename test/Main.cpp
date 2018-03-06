#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "../include/QuickTerminal.h"
#include "../include/AdvancedTabCompleter.h"
#include "../include/TerminalGrapth.h"

using namespace std;

string currentDate(){
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

    terminal::instance()->writeMessage("Having message: "+message);
    if(lmessage == "exit" || lmessage == "q" || lmessage == "quit"){
        terminal::instance()->writeMessage("§aExit program");
        terminal::uninstall();
    }
}

using namespace terminal;
using namespace std::chrono;

int main(int argsSize, char** args){
    terminal::install();
    writeMessage("§bHello §aWorld");
    terminal::uninstall();
    return 0;
    /*
    ArgumentParser parser;
    int number;
    parser.addArgument(ArgumentTypes::IntegerArgument("n", "number", ArgumentTypes::IntegerArgument::VALIDATOR_POSITIVE_OR_INF, &number, "A random number"));
    const char* message[] = {"-n", "\"1\""};
    cout << "X -> " << parser.parseArguments(2, (char**) message) << endl;
    cout << "Y -> " << number << endl;
    if(true) return 0;
     */

    tab::CompleterBase cmp;
    cmp.parameter("hello")->parameter("hell!");
    cmp.parameter("hello")->parameter("World");
    cmp.parameter("world")->parameter("World");
    cmp.wildcard()->parameter("xxxx");
    terminal::instance()->addTabCompleter(cmp.getBasedCompleter());

    terminal::instance()->writeMessage("§aHello §bworld § X §a");

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
    for (auto &it : vec)
        writeMessage(it);
    delete out;

    int time = 0;

    auto last = system_clock::now();
    while (terminal::active()){
        if(terminal::instance()->linesAvailable() > 0){
            time = 0;
            string message = terminal::instance()->readLine();
            handleLine(message);
            continue;
        }

        std::string stime = to_string(time / 1000);

        terminal::instance()->setPrompt(
                "[" + string(time > 10 * 1000 ? time > 30 * 1000 ? ANSI_RED : ANSI_BROWN : ANSI_GREEN) + stime +
                ANSI_RESET"] > ");
        //terminal::instance()->setPromt("["+getDate()+"] > ");
        std::this_thread::sleep_for(std::chrono::microseconds(123));
        time++;

        if(system_clock::now() - last > seconds(1)){
            last = system_clock::now();
            //writeMessage("Hello world");
        }
    }

    return 0;
}
