//
// Created by wolverindev on 04.12.16.
//

#include "../../include/CString.h"
#include "../../include/QuickTerminal.h"
#include <sstream>

using namespace std;

int CChar::getColor() {
    return this->attributes & 0x1F;
}

void CChar::setColor(int color) {
    uint8_t attr = this->attributes >> 5;
    this->attributes = color | (attr << 5);
}

bool CChar::hasStyle(int type) {
    return ((this->attributes >> (type + 5)) & 0x01) == 1;
}

void CChar::setStyle(int type, bool active) {
    if(active)
        this->attributes |= (1U << (type + 5));
    else
        this->attributes &= ~(1U << (type + 5));
}

void writeComplete(CChar& c,std::stringstream &ss){
    if(c.getColor() < 17)
        ss << "§" << std::hex << c.getColor() << std::dec;
    else
        ss << "§r";
    if(c.hasStyle(STYLE_MAGIC))
        ss << "§k";
    if(c.hasStyle(STYLE_BOLD))
        ss << "§l";
    if(c.hasStyle(STYLE_UNDERLINED))
        ss << "§n";
    if(c.hasStyle(STYLE_ITALIC))
        ss << "§m";
    if(c.hasStyle(STYLE_CURSIVE))
        ss << "§o";
    ss << c._char;
}

void CChar::append(std::stringstream &ss, CChar *prev) {
    if(prev == nullptr){
        writeComplete(*this, ss);
        return;
    }
    if(prev->getColor() != this->getColor()){
        writeComplete(*this, ss);
        return;
    }
    if(hasStyle(STYLE_MAGIC)){
        if(!prev->hasStyle(STYLE_MAGIC)) {
            ss << "§k";
        }
    }else{
        if(prev->hasStyle(STYLE_MAGIC)){
            writeComplete(*this, ss);
            return;
        }
    }
    if(hasStyle(STYLE_UNDERLINED)){
        if(!prev->hasStyle(STYLE_UNDERLINED)) {
            ss << "§n";
        }
    }else{
        if(prev->hasStyle(STYLE_UNDERLINED)){
            writeComplete(*this, ss);
            return;
        }
    }
    if(hasStyle(STYLE_ITALIC)){
        if(!prev->hasStyle(STYLE_ITALIC)) {
            ss << "§m";
        }
    }else{
        if(prev->hasStyle(STYLE_ITALIC)){
            writeComplete(*this, ss);
            return;
        }
    }
    if(hasStyle(STYLE_CURSIVE)){
        if(!prev->hasStyle(STYLE_CURSIVE)) {
            ss << "§o";
        }
    }else{
        if(prev->hasStyle(STYLE_CURSIVE)){
            writeComplete(*this, ss);
            return;
        }
    }
    if(hasStyle(STYLE_BOLD)){
        if(!prev->hasStyle(STYLE_BOLD)) {
            ss << "§o";
        }
    }else{
        if(prev->hasStyle(STYLE_BOLD)){
            writeComplete(*this, ss);
            return;
        }
    }
    ss << this->_char;
}

CString::CString(CString& other) {
    for(auto it = other.chars.begin(); it != other.chars.end(); it++)
        this->chars.push_back(*it);
    writeMessage("Char size: "+to_string(this->chars.size()));
}

CString::CString() {}



CString::CString(std::string str) {
    CChar* last = nullptr;
    CChar current;

    for(int i = 0;i<str.length();i++){
        if(last != nullptr)
            current.attributes = last->attributes;

        if((unsigned char) str[i] == 194 && (unsigned char) str[i+1] == 167){
            i++;
            char code = tolower(str[++i]);
            if(code >= '0' && code <= '9' || code >= 'a' && code <= 'f'){
                current.attributes = code > '9' ? code - 'a' + 10 : code - '0';
                last = &current;
                continue;
            }
            else {
                if(code == 'k'){
                    current.setStyle(STYLE_MAGIC, true);
                    last = &current;
                    continue;
                }
                else if(code == 'l'){
                    current.setStyle(STYLE_BOLD, true);
                    last = &current;
                    continue;
                }
                else if(code == 'm'){
                    current.setStyle(STYLE_ITALIC, true);
                    last = &current;
                    continue;
                }
                else if(code == 'n'){
                    current.setStyle(STYLE_UNDERLINED, true);
                    last = &current;
                    writeMessage("Set n: "+to_string(current.getColor()));
                    continue;
                }
                else if(code == 'o'){
                    current.setStyle(STYLE_CURSIVE, true);
                    last = &current;
                    continue;
                }
                else if(code == 'r'){
                    current.attributes = 17;
                    last = &current;
                    continue;
                }
                else{
                    writeMessage("Code: "+code);
                    i-=2;
                }
            }
        }
        current._char = str[i];
        this->chars.push_back(current);
        last = &current;
    }
}
CString::CString(std::vector<CChar>& chars) {
    for(auto it = chars.begin(); it != chars.end(); it++)
        this->chars.push_back(*it);
}

/*
CString& CString::operator+(const CString& other) {
    writeMessage("Other size: "+to_string(other.chars.size()));
    writeMessage("Own: "+to_string(chars.size()));
    CString _new(*this);
    writeMessage("Copy: "+_new.str());
    CString& out = _new.operator+=(other);
    writeMessage("Copy1: "+out.str());
    return out;
}
*/

CString& CString::operator+=(const CString& other) {
    for(auto it = other.chars.begin(); it != other.chars.end(); it++)
        this->chars.push_back(*it);
    return *this;
}
CString& CString::operator+=(const CChar _char) {
    this->chars.push_back(_char);
    return *this;
}

CString& CString::operator+=(const char* chars) {
    this->operator+=(CString(string(chars)));
    return *this;
}

CChar& CString::operator[](const int index) {
    return this->chars[index];
}

std::string CString::str() {
    stringstream ss;
    CChar* prev = nullptr;

    for(auto it = this->chars.begin(); it != this->chars.end(); it++){
        it->append(ss, prev);
        prev = &(*it);
    }
    return ss.str();
}