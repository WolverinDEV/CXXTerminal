//
// Created by wolverindev on 17.10.16.
//
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <functional>
#include "QuickTerminal.h"

using namespace std;

class Argument {
    public:
        string shortKey;
        string longKey;
        string discription;
        function<bool(string)> applay;
        function<string()> defaultValue;
};

namespace ArgumentTypes {
        typedef std::function<string()> DefaultValueFunction;
        extern std::function<DefaultValueFunction(string)> CAPTURE_VALUE;

        const DefaultValueFunction UNKNOWN_DEFAULT_VALUE = NULL;
        /*
        [](){
            return "unknown";
        };
         */

        typedef std::function<bool(bool*)> BoolValidator;
        const BoolValidator ALL_BOOL_ARG_TESTER = [](bool*){
            return 1;
        };

        class BoolArgument : public Argument {
            public:
                BoolArgument(string shortArg, string longArg, BoolValidator validator, bool* var, string description) {
                    Argument::discription = description;
                    Argument::longKey = longArg;
                    Argument::shortKey = shortArg;
                    applay = [var, longArg, validator](string value){
                        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

                        bool number = (value.compare("true") == 0 || value.compare("1") == 0 || value.compare("t") == 0) ? 1 : 0;
                        if(!validator(&number)){
#ifdef DEBUG
                            writeMessage("[DEBUG] Dont use val "+to_string(number)+" for "+longArg);
#endif
                            return 0;
                        }
#ifdef DEBUG
                        writeMessage("Changing "+longArg+" to "+(number == 1 ? "true" : "false"));
#endif
                        *var = number;
                    };
                    const bool defaultVal = *var;
                    this->defaultValue = [defaultVal]{
                        return defaultVal ? "true" : "false";
                    };
                };
        };

        typedef std::function<bool(int*)> IntegerValidator;

        class IntegerArgument : public Argument {
            public:
                IntegerArgument(string shortArg, string longArg, IntegerValidator validator, int* var, string description) {
                    Argument::discription = description;
                    Argument::longKey = longArg;
                    Argument::shortKey = shortArg;
                    applay = [var, longArg, validator](string value){
                        if(value.find_first_not_of("-1234567890") != std::string::npos){
                            writeMessage("§cInvalid argument! Argument '"+longArg+"' isnt an valid number!");
                            return 0;
                        }
                        int number = atoi(value.c_str());
                        if(!validator(&number)){
#ifdef DEBUG
                            debugMessage("[DEBUG] Dont use val: "+to_string(number)+" for "+longArg);
#endif
                            return 0;
                        }
                        *var = number;
                    };

                    const int defaultVal = *var;
                    this->defaultValue = [defaultVal](){
                        return to_string(defaultVal);
                    };
                };

                IntegerArgument(string shortArg, string longArg, int min, int max, int* var, string description) : IntegerArgument(shortArg, longArg, (IntegerValidator) [longArg, min, max](int* number){
                    if(*number < min){
                        writeMessage("§cInvalid argument! Argument '"+longArg+"' is smaler than "+to_string(min));
                        return 0;
                    }
                    if(*number > max){
                        writeMessage("§cInvalid argument! Argument '"+longArg+"' is bigger than "+to_string(min));
                        return 0;
                    }
                    return 1;
                }, var, description) {};
        };

        const IntegerValidator ALL_INT_ARG_TESTER = [](int*){
            return 1;
        };

        const IntegerValidator INFINITY_INT_ARG_TESTER = [](int* number){
            if(*number < 0 && *number != -1){
                writeMessage("§cInvalid argument! Integer is negative!");
                return 0;
            }
            return 1;
        };
}

class ArgumentParser {
    public:
        void addArgument(Argument arg){
            arguments.push_back(arg);
        }

        bool parseArguments(int argc, char** argv);

        bool applayArgument(string key, string value);

        void printHelp(string error = "");

        vector<Argument> getArguments(){
            return this->arguments;
        }
    private:
        vector<Argument> arguments;
};
