//
// Created by wolverindev on 22.11.16.
//

#ifndef CXXTERMINAL_ADVANCEDTABCOMPLETER_H
#define CXXTERMINAL_ADVANCEDTABCOMPLETER_H

#include "Terminal.h"
#include <string>
#include <vector>

namespace Terminal {
        class AvTabCompleter {
            public:
                AvTabCompleter();
                std::vector<std::string> getAvaribilities(int index);

                void addParameter(int index, std::string parm);
                void unregister(int index, std::string parm);

                TabCompleter& getBasedCompleter(){
                    return this->baseCompleter;
                }

                bool isMatchStart(){
                    return suggestOnStartMath;
                }
                void setMatchOnStart(bool flag){
                    this->suggestOnStartMath = flag;
                }
            private:
                TabCompleter baseCompleter = NULL;

                bool suggestOnStartMath = true;

                void pushbackArg(int index, std::string& str);

                std::vector<std::vector<std::string>> parms;
        };
}


#endif //CXXTERMINAL_ADVANCEDTABCOMPLETER_H
