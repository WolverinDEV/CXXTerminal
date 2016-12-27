//
// Created by wolverindev on 22.11.16.
//

#include "../include/AdvancedTabCompleter.h"
#include "../include/QuickTerminal.h"
#include <cstring>
#include <algorithm>

using namespace std;
namespace Terminal {
        inline void split(const string &s, string delim, vector<string> &elems) {
            auto start = 0U;
            auto end = s.find(delim);
            while (end != std::string::npos) {
                elems.push_back(s.substr(start, end - start));
                start = end + delim.length();
                end = s.find(delim, start);
            }
            elems.push_back(s.substr(start, end - start));
        }

        AvTabCompleter::AvTabCompleter() : DependCompleter(nullptr, "", false) {
            this->baseCompleter = [&](std::string line, std::string buffer, std::vector<std::string> &completions) {
                vector<string> args;
                split(line, " ", args);

                vector<DependCompleter *> completer = getAvaribilities(args/*, spaces*/);

                for (auto it = completer.begin(); it != completer.end(); it++) {
                    completions.push_back((*it)->expected);
                }
            };
        }
        DependCompleter::DependCompleter(std::string parm, bool ignoreCase) : DependCompleter(nullptr, parm,
                                                                                              ignoreCase) {}

        DependCompleter::DependCompleter(DependCompleter *parent, std::string parm, bool ignoreCase) {
            this->root = parent;
            this->expected = parm;
            this->ignoreCase = ignoreCase;
        }

        DependCompleter::~DependCompleter() {
            while (!this->next.empty()){
                delete next[0];
            }
            if(root)
                root->next.erase(std::find(root->next.begin(), root->next.end(), this)/*, root->next.end()*/);
        }

        std::vector<DependCompleter *> DependCompleter::getNext() {
            return this->next;
        }

        bool DependCompleter::accept(std::string in) {
            if (in.size() > this->expected.size())
                return false;

            string cin(in);
            string check = this->expected.substr(0, in.size());
            if (ignoreCase) {
                std::transform(cin.begin(), cin.end(), cin.begin(), ::tolower);
                std::transform(check.begin(), check.end(), check.begin(), ::tolower);
            }
            return cin.compare(check) == 0;
        }

        bool DependCompleter::acceptExact(std::string in) {
            if (in.size() != this->expected.size())
                return false;
            string cin(in);
            string check(this->expected);
            if (ignoreCase) {
                std::transform(cin.begin(), cin.end(), cin.begin(), ::tolower);
                std::transform(check.begin(), check.end(), check.begin(), ::tolower);
            }
            return cin.compare(check) == 0;
        }

        DependCompleter *DependCompleter::parameter(std::string parameter) {
            //TODO Better implementation of if item alredy registered..
            for (auto it = this->next.begin(); it != this->next.end(); it++)
                if ((*it)->expected == parameter)
                    return *it;
            DependCompleter *next = new DependCompleter(this, parameter, true);
            this->next.push_back(next);
            return next;
        }

        bool DependCompleter::unregister(std::string parameter) {
            for (auto it = this->next.begin(); it != this->next.end(); it++)
                if ((*it)->expected == parameter){
                    DependCompleter*& completer = *it;
                    writeMessage("Y: "+to_string(this->next.size()));
                    //this->next.erase(it);
                    delete completer;

                    writeMessage("DELET!: "+to_string(this->next.size()));
                    return true;
                }
            return false;
        }

        DependCompleter *DependCompleter::wildcard() {
            for (auto it = this->next.begin(); it != this->next.end(); it++)
                if (dynamic_cast<WildcardCompleter *>(*it) != nullptr)
                    return *it;
            DependCompleter *next = new WildcardCompleter(this);
            this->next.push_back(next);
            return next;
        }

        bool DependCompleter::unregisterWildcard() {
            for (auto it = this->next.begin(); it != this->next.end(); it++)
                if (dynamic_cast<WildcardCompleter *>(*it) != nullptr){
                    //this->next.erase(std::find(this->next.begin(), this->next.end(), *it), this->next.end());
                    delete *it; //Automaticly get removed
                    return true;
                }
            return false;
        }

        std::vector<DependCompleter *> AvTabCompleter::getAvaribilities(std::vector<std::string> &args/*, int index*/) {
            vector<DependCompleter *> current(this->next);

            for (int i = 0; i < args.size() - 1; i++) {
                vector<DependCompleter *> out;
                string str = args.size() > i ? args[i] : "";
                for (auto it = current.begin(); it != current.end(); it++) {
                    if ((*it)->acceptExact(str)) {
                        vector<DependCompleter *> c = (*it)->getNext();
                        for (auto i2 = c.begin(); i2 != c.end(); i2++)
                            out.push_back(*i2);
                    }
                }
                current = vector<DependCompleter *>(out);
            }

            vector<DependCompleter *> out;
            for (auto it = current.begin(); it != current.end(); it++) {
                if ((*it)->accept(args[args.size() - 1]) && (*it)->expected != "") {
                    out.push_back(*it);
                }
            }

            return out;
        }

        WildcardCompleter::WildcardCompleter(DependCompleter *parent) : DependCompleter(parent, "", true) {}

        bool WildcardCompleter::accept(std::string in) {
            return true;
        }

        bool WildcardCompleter::acceptExact(std::string in) {
            return true;
        }
}