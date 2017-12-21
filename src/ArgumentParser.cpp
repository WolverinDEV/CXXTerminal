#include <sstream>
#include "../include/ArgumentParser.h"
using namespace terminal::arguments;

bool ArgumentParser::parseArguments(int argc, char **argv) {
    bool inQuote = false;
    bool qouteRemoveFirstSpace = false;
    string currentQuote = "";
    string parameter = "";

    for (int i = 0; i < argc; i++) {
        string current = string(argv[i]);

        handleStuff:
        if(current.find_first_not_of(' ') == string::npos) continue;

        if ((parameter == "" || current[0] == '-') && !inQuote) {
            if (parameter != "") {
                if (!applayArgument(parameter, "true")) {
                    printHelp("Cant find argument '" + parameter + "'");
                    return false;
                }
                parameter = "";
            }
            if (current[0] != '-') {
                printHelp("Invalid key '" + current + "'");
                return false;
            }
            parameter = current.substr(current[1] == '-' ? 2 : 1);
            if (i + 1 == argc) {
                if (!applayArgument(parameter, "true")) {
                    printHelp("Cant find argument '" + parameter + "'");
                    return false;
                }
            }
        } else {
            if (inQuote) {
                if (current[current.length() - 1] == '"') {
                    inQuote = false;
                    currentQuote += " " + current.substr(0, current.length() - 1);
                    if(qouteRemoveFirstSpace){
                        qouteRemoveFirstSpace = false;
                        currentQuote = currentQuote.substr(1);
                    }
                    if (!applayArgument(parameter, currentQuote)) {
                        printHelp("Cant find argument '" + parameter + "'");
                        return false;
                    }
                    currentQuote = "";
                    parameter = "";
                    continue;
                } else {
                    currentQuote += " " + current.substr(0, current.length());
                }
            } else if (current[0] == '"') {
                inQuote = true;
                qouteRemoveFirstSpace = true;
                currentQuote = "";
                current = current.substr(1);
                goto handleStuff; //handle the rest of the text maybe it has another quote at the end
            } else {
                if (!applayArgument(parameter, current)) {
                    printHelp("Cant find argument '" + parameter + "'");
                    return false;
                }
                parameter = "";
            }
        }
    }
    if (inQuote) {
        writeMessage("§cAn error happend while parsing arguments. Quotes opened but never closed");
        return false;
    }

    return true;
}


string fill(string in, string fill, int size, int bounded = 1) {
    string out = in;
    while (out.size() < size) {
        switch (bounded) {
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
    if (error != "") {
        writeMessage("§cAn error happend while parsing arguments. Message: " + error);
    }
    writeMessage("Arguments:");
    size_t bounds[3];
    for (int i = 0; i < 3; i++)
        bounds[i] = 0;

    for (vector<Argument>::iterator it = arguments.begin(); it != arguments.end(); it++) {
        if (it->shortKey != "" && bounds[0] < it->shortKey.size())
            bounds[0] = it->shortKey.size();
        if (it->longKey != "" && bounds[1] < it->longKey.size())
            bounds[1] = it->longKey.size();
        if (it->description != "" && bounds[2] < it->description.size())
            bounds[2] = it->description.size();
    }
    for (vector<Argument>::iterator it = arguments.begin(); it != arguments.end(); it++) {
        stringstream ss;
        ss << "  -" + fill(it->shortKey, " ", bounds[0], 1) + "  or  --" + fill(it->longKey, " ", bounds[1], 1);
        ss << "  |   " + fill(it->description, " ", bounds[2], 1);
        if (it->defaultValue != NULL)
            ss << " Default value: " + it->defaultValue();
        writeMessage(ss.str());
    }
}

bool ArgumentParser::applayArgument(string key, string value) {
    for (vector<Argument>::iterator it = arguments.begin(); it != arguments.end(); it++) {
        if ((*it).shortKey.compare(key) == 0 || (*it).longKey.compare(key) == 0) {
            if (it->applay != nullptr)
                if (!(*it).applay(value))
                    writeMessage("§cInvalid value for argument '" + key + "'. Recived value: '" + value + "'");
            return 1;
        }
    }
    return 0;
}

Argument::Argument(const string &shortKey, const string &longKey, const string &description, function<bool(string)> applay, function<string()> defaultValue) : shortKey(shortKey), longKey(longKey), description(description),
                                                                                                                                                               applay(applay), defaultValue(defaultValue) {}

using namespace ArgumentTypes;
const BoolValidator BoolArgument::ALL_BOOL_ARG_TESTER = [](bool *) { return true; };

const IntegerValidator IntegerArgument::VALIDATOR_ALL = [](int *) { return true; };
const IntegerValidator IntegerArgument::VALIDATOR_POSITIVE = [](int *number) { return *number >= 0; };
const IntegerValidator IntegerArgument::VALIDATOR_POSITIVE_OR_INF = [](int *number) { return IntegerArgument::VALIDATOR_POSITIVE(number) && *number != -1; };