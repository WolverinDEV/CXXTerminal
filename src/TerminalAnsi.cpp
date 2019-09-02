#include "../include/Terminal.h"
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;
using namespace terminal;

const static size_t ansi_code_length = 22;
const static char ansi_code_keys[]{"abcdefknmolr0123456789ABCDEFKNMOLR"};
const static char* ansi_code_values[]{
    /* A-F */
    ANSI_LIGHT_GREEN,
    ANSI_LIGHT_CYAN,
    ANSI_LIGHT_RED,
    ANSI_LIGHT_PURPLE,
    ANSI_YELLOW,
    ANSI_WHITE,
    /* K N M O L R */
    ANSI_UNDERLINE,
    ANSI_UNDERLINE,
    (ANSI_UNDERLINE ANSI_REVERSE),
    ANSI_BOLD,
    ANSI_BOLD,
    ANSI_RESET,

    /* 0-9 */
    ANSI_BLACK,
    ANSI_BLUE,
    ANSI_GREEN,
    ANSI_CYAN,
    ANSI_RED,
    ANSI_PURPLE,
    ANSI_BROWN,
    ANSI_GRAY,
    ANSI_DARK_GREY,
    ANSI_LIGHT_BLUE,
};
string terminal::parseCharacterCodes(string in, std::string characterCode) {
    string response;
    response.reserve(in.length() + 64);

    if(characterCode.empty())
        characterCode = "§";

    int index = 0, found_index;
    while((found_index = in.find(characterCode, index)) != -1) {
        if(index != found_index)
            response.append(in, index, found_index - index);
        index = found_index;
        if(found_index + characterCode.length() > in.length())
            break;
        auto key_ptr = memchr(ansi_code_keys, in[found_index + characterCode.length()], sizeof(ansi_code_keys));
        if(!key_ptr) {
        	response.append(characterCode);
	        index += characterCode.length();
	        continue;
        }
        size_t code_index = (uintptr_t) key_ptr - (uintptr_t) ansi_code_keys;
        if(code_index > ansi_code_length)
            code_index -= ansi_code_length;
        if(code_index > ansi_code_length) {
	        response.append(characterCode);
	        index += characterCode.length();
	        continue;
        }
        response.append(ansi_code_values[code_index]);
        index += characterCode.length() + 1;
    }

    if(index != found_index)
        response.append(in, index, found_index - index);
    response.shrink_to_fit();
    return response;
}

std::string terminal::stripCharacterCodes(std::string in, std::string characterCode) {
    string response;
    response.reserve(in.length());

    if(characterCode.empty())
        characterCode += "§";

    int index = 0, found_index;
    while((found_index = in.find(characterCode, index)) != -1) {
        if(index != found_index)
            response.append(in, index, found_index - index);
        index = found_index;
        if(found_index + characterCode.length() > in.length())
            break;
        auto key_ptr = memchr(ansi_code_keys, in[found_index + characterCode.length()], sizeof(ansi_code_keys));
        if(!key_ptr) {
	        response.append(characterCode);
	        index += characterCode.length();
	        continue;
        }
        size_t code_index = (uintptr_t) key_ptr - (uintptr_t) ansi_code_keys;
        if(code_index > ansi_code_length)
            code_index -= ansi_code_length;
        if(code_index > ansi_code_length) {
	        response.append(characterCode);
	        index += characterCode.length();
	        continue;
        }
        index += characterCode.length() + 1;
    }

    if(index != found_index)
        response.append(in, index, found_index - index);
    response.shrink_to_fit();
    return response;
}