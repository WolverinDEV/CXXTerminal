#pragma once

#include <string>
#include <vector>

#define STYLE_MAGIC 0
#define STYLE_BOLD 1
#define STYLE_ITALIC 2
#define STYLE_UNDERLINED 3
#define STYLE_CURSIVE 4

struct CChar {
    char _char = -1;
    uint16_t attributes = 17;

    int color();
    CChar& color(int color);

    bool hasStyle(int type);
    CChar& setStyle(int type, bool active);

    void append(std::stringstream&,CChar*);
};

class CString{
    public:
        CString(const CString&);
        CString();
        CString(std::string);
        CString(std::vector<CChar>&);
        //CString& operator+(const CString&);
        CString& operator+=(const CString&);
        CString& operator+=(const CChar);
        CString& operator+=(const char*);
        CChar&operator[](const int);

        std::string str();
    //private:
        std::vector<CChar> chars;
};
