//
// Created by wolverindev on 27.12.16.
//

#include <sstream>
#include "../include/ArgumentParser.h"

string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

bool ArgumentParser::parseArguments(int argc, char **argv) {
    bool inQuote = false;
    string currentQuote = "";
    string parameter = "";

    for(int i = 0;i<argc;i++){
        string current = string(argv[i]);
        if(replaceAll(current, " ","") == "")
            continue;
        if((parameter == "" || current[0] == '-') && !inQuote){
            if(parameter != ""){
                if(!applayArgument(parameter, "true")){
                    printHelp("Didnt find argument "+parameter);
                    return false;
                }
                parameter = "";
            }
            if(current[0] != '-'){
                printHelp("Invalid key '"+current+"'");
                return false;
            }
            parameter = current.substr(current[1] == '-' ? 2 : 1);
            if(i+1 == argc){
                if(!applayArgument(parameter, "true")){
                    printHelp("Didnt find argument "+parameter);
                    return false;
                }
            }
        } else {
            if(inQuote){
                if(current[current.length()-1] == '"'){
                    inQuote = false;
                    currentQuote += " " + current.substr(0, current.length()-1);
                    if(!applayArgument(parameter, currentQuote)){
                        printHelp("Didnt find argument "+parameter);
                        return false;
                    }
                    currentQuote = "";
                    parameter = "";
                    continue;
                }
                else {
                    currentQuote += " " + current.substr(0, current.length());
                }
            }
            else if(current[0] == '"'){
                inQuote = true;
                currentQuote = current.substr(1);
                continue;
            }
            else {
                if(!applayArgument(parameter, current)){
                    printHelp("Didnt find argument "+parameter);
                    return false;
                }
                parameter = "";
            }
        }
    }
    return true;
}


string fill(string in,string fill, int size, int bounded = 1){
    string out = in;
    while (out.size() < size){
        switch (bounded){
            case -1:
                out = fill + out;
                break;
            case 0:
                out = fill + out + fill;
                break;
            case 1:
                out += fill;
                break;
        }
    }
    return out;
}

void ArgumentParser::printHelp(string error) {
    if(error != ""){
        writeMessage("§cAn error happend while parsing arguments. Message: "+error);
    }
    writeMessage("Arguments:");
    int bounds[3];
    for(int i = 0;i<3;i++)
        bounds[i] = 0;

    for(vector<Argument>::iterator it = arguments.begin(); it != arguments.end(); it++){
        if(it->shortKey != "" && bounds[0] < it->shortKey.size())
            bounds[0] = it->shortKey.size();
        if(it->longKey != "" && bounds[1] < it->longKey.size())
            bounds[1] = it->longKey.size();
        if(it->discription != "" && bounds[2] < it->discription.size())
            bounds[2] = it->discription.size();
    }
    for(vector<Argument>::iterator it = arguments.begin(); it != arguments.end(); it++){
        stringstream ss;
        ss << "  -"+fill(it->shortKey, " ", bounds[0], 1)+"  or  --"+fill(it->longKey, " ", bounds[1], 1);
        ss << "  |   "+fill(it->discription, " ", bounds[2], 1);
        if(it->defaultValue != NULL)
            ss << " Default: "+it->defaultValue();
        writeMessage(ss.str());
    }
}

bool ArgumentParser::applayArgument(string key, string value) {
    for(vector<Argument>::iterator it = arguments.begin(); it != arguments.end(); it++){
        if((*it).shortKey.compare(key) == 0 || (*it).longKey.compare(key) == 0) {
            if(it->applay != nullptr)
                (*it).applay(value);
            return 1;
        }
    }
    return 0;
}

std::function<ArgumentTypes::DefaultValueFunction(string)> ArgumentTypes::CAPTURE_VALUE = [](string str){
    return [str](){ //get captured
        return str;
    };
};