//
// Created by wolverindev on 22.11.16.
//

#include "../include/AdvancedTabCompleter.h"
#include <cstring>
#include <algorithm>

using namespace std;
namespace Terminal {
    inline void split(const string &s, string delim, vector<string> &elems) {
        auto start = 0U;
        auto end = s.find(delim);
        while (end != std::string::npos)
        {
            elems.push_back(s.substr(start, end - start));
            start = end + delim.length();
            end = s.find(delim, start);
        }
        elems.push_back(s.substr(start, end - start));
    }

    AvTabCompleter::AvTabCompleter() : DependCompleter(nullptr, "", false){
        this->baseCompleter = [&](std::string line, std::string buffer, std::vector<std::string> &completions) {
            int spaces = 0;
            for (auto it = line.begin(); it != line.end(); it++)
                if (*it == ' ')
                    spaces++;

            if (parms.size() > spaces) { //?
                vector<string> args;
                split(line, " ", args);

                vector<DependCompleter*> completer = getAvaribilities(args, spaces);

                for(auto it = completer.begin(); it != completer.end(); it++){

                }
                std::vector<std::string> av = parms[spaces];
                for (auto it = av.begin(); it != av.end(); it++) {
                    if (this->suggestOnStartMath) {
                        std::string &str = *it;
                        if (str.size() > buffer.size()) {
                            std::string lower = str.substr(0, buffer.size());
                            std::string sec = buffer;
                            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                            std::transform(sec.begin(), sec.end(), sec.begin(), ::tolower);
                            if (lower.compare(sec) == 0)
                                completions.push_back(*it);
                        } else {} //Its another word with a bigger length
                    } else
                        completions.push_back(*it);
                }
            }
        };
    }

    /*
        void AvTabCompleter::addParameter(int index, std::string parm) {
            if(!(this->parms.size() > index))
                this->parms.resize(index+1);
            this->parms[index].push_back(parm);
        }
*/

    DependCompleter::DependCompleter(std::string parm, bool ignoreCase) : DependCompleter(nullptr, parm, ignoreCase) { }

    DependCompleter::DependCompleter(DependCompleter *parent, std::string parm, bool ignoreCase) {
        this->root = parent;
        this->expected = parm;
        this->ignoreCase = ignoreCase;
    }

    std::vector<DependCompleter*> DependCompleter::getNext() {
        return this->next;
    }

    bool DependCompleter::accept(std::string in) {
        if(in.size() > this->expected.size())
            return false;

        string cin(in);
        string check = this->expected.substr(0, in.size());
        if(ignoreCase){
            std::transform(cin.begin(), cin.end(), cin.begin(), ::tolower);
            std::transform(check.begin(), check.end(), check.begin(), ::tolower);
        }
        return cin.compare(check) == 0;
    }

    DependCompleter* DependCompleter::parameter(std::string parameter) {
        //TODO check if alredy registerted!
        DependCompleter next(this, parameter, true);
        this->next.push_back(&next);
        return &next;
    }


    std::vector<DependCompleter*> AvTabCompleter::getAvaribilities(std::vector<std::string> &args, int index) {
        vector<DependCompleter*> current(this->parms);

        for(int i = 0;i<index; i++){
            vector<DependCompleter*> out;
            string str = args.size() > i ? args[i] : "";

            for(auto it = current.begin(); it != current.end(); it++){
                if((*it)->accept(str)){
                    vector<DependCompleter*> c = (*it)->getNext();
                    for(auto i2 = c.begin(); i2 != c.end(); i2++)
                        current.push_back(*i2);
                }
            }
            current = out;
        }
    }

    DependCompleter* AvTabCompleter::addParameter(int index, std::string parm) {
        DependCompleter completer(parm);

        //TODO index...
        parms.push_back(&parm);
        return &completer;
    }
}