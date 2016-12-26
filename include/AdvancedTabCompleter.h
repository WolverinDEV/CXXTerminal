//
// Created by wolverindev on 22.11.16.
//

#ifndef CXXTERMINAL_ADVANCEDTABCOMPLETER_H
#define CXXTERMINAL_ADVANCEDTABCOMPLETER_H

#include "Terminal.h"
#include <string>
#include <vector>

namespace Terminal {
        class DependCompleter {
            public:
                DependCompleter(std::string, bool ignoreCase = true);
                DependCompleter(DependCompleter*,std::string, bool ignoreCase = true);

                DependCompleter* parameter(std::string parameter);
                DependCompleter* wildcard();

                virtual bool accept(std::string in);

                std::vector<DependCompleter*> getNext();
            private:
                DependCompleter* root;
                std::vector<DependCompleter*> next;

                bool ignoreCase;
                std::string expected;
        };
        class WildcardCompleter : public DependCompleter {

        };
    class
        class AvTabCompleter : public DependCompleter{
            public:
                AvTabCompleter();
                std::vector<DependCompleter*> getAvaribilities(std::vector<std::string>& args, int index);

                //DependCompleter* addParameter(int index = 0, std::string parm);
                //void unregister(int index, std::string parm);
                //void unregister(int index, DependCompleter* parm);

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

                std::vector<DependCompleter*> parms;
        };
}


#endif //CXXTERMINAL_ADVANCEDTABCOMPLETER_H
