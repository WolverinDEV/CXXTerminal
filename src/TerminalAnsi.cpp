//
// Created by wolverindev on 20.12.17.
//
#include "../include/Terminal.h"
#include <sstream>

using namespace std;

using namespace terminal;
string terminal::parseCharacterCodes(string in, std::string characterCode) {
    if(characterCode.empty()) characterCode += '§';

    stringstream out;
    size_t index = 0;
    size_t oldIndex = 0;
    while((index = in.find(characterCode, oldIndex)) > 0 && index < in.size()){
        out << in.substr(oldIndex, index - oldIndex);
        switch (tolower(in.substr(characterCode.length() + 1, 1)[0])){
            case '0':
                out << ANSI_BLACK; break;
            case '1':
                out << ANSI_BLUE; break;
            case '2':
                out << ANSI_GREEN; break;
            case '3':
                out << ANSI_CYAN; break;
            case '4':
                out << ANSI_RED; break;
            case '5':
                out << ANSI_PURPLE; break;
            case '6':
                out << ANSI_BROWN; break;
            case '7':
                out << ANSI_GRAY; break;
            case '8':
                out << ANSI_DARK_GREY; break;
            case '9':
                out << ANSI_LIGHT_BLUE; break;
            case 'a':
                out << ANSI_LIGHT_GREEN; break;
            case 'b':
                out << ANSI_LIGHT_CYAN; break;
            case 'c':
                out << ANSI_LIGHT_RED; break;
            case 'd':
                out << ANSI_LIGHT_PURPLE; break;
            case 'e':
                out << ANSI_YELLOW; break;
            case 'f':
                out << ANSI_WHITE; break;
            case 'k': break;
            case 'n':
                out << ANSI_UNDERLINE; break;
            case 'm':
                out << ANSI_UNDERLINE << ANSI_REVERSE; break;
            case 'o': break;
            case 'l':
                out << ANSI_BOLD; break;
            case 'r':
                out << ANSI_RESET; break;
            default:
                out << "§";
                index -= 1;
        }
        index += characterCode.length() + 1;
        oldIndex = index;
    }
    out << in.substr(oldIndex);
    return out.str();
}

std::string terminal::stripCharacterCodes(std::string in, std::string characterCode) {
    if(characterCode.empty()) characterCode = "§";

    stringstream out;
    size_t index = 0;
    size_t oldIndex = 0;
    while((index = in.find(characterCode, oldIndex)) > 0 && index < in.size()){
        out << in.substr(oldIndex, index - oldIndex);
        switch (tolower(in.substr(characterCode.length() + 1, 1)[0])){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'k':
            case 'n':
            case 'm':
            case 'o':
            case 'l':
            case 'r':
                break;
            default:
                out << characterCode;
                index -= 1;
        }
        index += characterCode.length() + 1;
        oldIndex = index;
    }
    out << in.substr(oldIndex);
    return out.str();
}