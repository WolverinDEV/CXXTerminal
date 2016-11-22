//
// Created by wolverindev on 22.11.16.
//

#include "../include/AdvancedTabCompleter.h"
#include <cstring>
#include <algorithm>

namespace Terminal {
        AvTabCompleter::AvTabCompleter() {
           this->baseCompleter = [&](std::string line,std::string buffer,std::vector<std::string>& completions){
                int spaces = 0;
                for(auto it = line.begin();it != line.end();it++)
                    if(*it == ' ')
                        spaces++;
                if(parms.size() > spaces){
                    std::vector<std::string> av = parms[spaces];
                    for(auto it = av.begin(); it != av.end(); it++){
                        if(this->suggestOnStartMath){
                            std::string& str = *it;
                            if(str.size() > buffer.size()){
                                std::string lower = str.substr(0, buffer.size());
                                std::string sec = buffer;
                                std::transform(lower.begin(),lower.end(), lower.begin(), ::tolower);
                                std::transform(sec.begin(),sec.end(), sec.begin(), ::tolower);
                                if(lower.compare(sec) == 0)
                                    completions.push_back(*it);
                            } else {} //Its another word with a bigger length
                        }
                        else
                            completions.push_back(*it);
                    }
                }
            };
        }

        void AvTabCompleter::addParameter(int index, std::string parm) {
            if(!(this->parms.size() > index))
                this->parms.resize(index+1);
            this->parms[index].push_back(parm);
        }
}