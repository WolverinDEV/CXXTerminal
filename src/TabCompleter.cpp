//
// Created by wolverindev on 22.11.16.
//

#include "../include/AdvancedTabCompleter.h"
#include "../include/QuickTerminal.h"
#include <cstring>
#include <algorithm>

using namespace std;
namespace terminal::tab {
    inline void split(const string &s, const string& delim, vector<string> &elems) {
        auto start = 0U;
        auto end = s.find(delim);
        while (end != std::string::npos) {
            elems.push_back(s.substr(start, end - start));
            start = static_cast<unsigned int>(end + delim.length());
            end = s.find(delim, start);
        }
        elems.push_back(s.substr(start, end - start));
    }

    CompleterBase::CompleterBase() : completer(nullptr, "", false) {
        this->baseCompleter = [&](std::string line, std::string buffer, std::vector<std::string> &completions) {
            vector<string> args;
            split(line, " ", args);

            vector<completer *> completer = availabilities(args);
            for (auto& it : completer)
                completions.push_back(it->expected);
        };
    }

    completer::completer(std::string parm, bool ignoreCase) : completer(nullptr, parm, ignoreCase) {}

    completer::completer(completer *parent, std::string parm, bool ignoreCase) {
        this->root = parent;
        this->expected = parm;
        this->ignoreCase = ignoreCase;
    }

    completer::~completer() {
        while (!this->_next.empty()) {
            delete _next.front();
        }
        if (root)
            root->_next.erase(std::find(root->_next.begin(), root->_next.end(), this));
    }

    std::vector<completer *> completer::next() {
        return this->_next;
    }

    bool completer::accept(std::string in) {
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

    bool completer::acceptExact(std::string in) {
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

    completer *completer::parameter(std::string parameter) {
        //TODO Better implementation of if item already registered..
        for (auto it = this->_next.begin(); it != this->_next.end(); it++)
            if ((*it)->expected == parameter)
                return *it;
        completer *next = new completer(this, parameter, true);
        this->_next.push_back(next);
        return next;
    }

    bool completer::unregister(std::string parameter) {
        for (auto it = this->_next.begin(); it != this->_next.end(); it++)
            if ((*it)->expected == parameter) {
                completer* completer = *it;
                //this->next.erase(it);
                delete completer;
                return true;
            }
        return false;
    }

    bool completer::registerTabCompleter(completer *completer) {
        for (auto it = this->_next.begin(); it != this->_next.end(); it++)
            if ((*it)->expected == completer->expected)
                return false;
        this->_next.push_back(completer);
        return true;
    }

    bool completer::unregisterTabCompleter(tab::completer *completer) {
        this->_next.erase(std::find(this->_next.begin(), this->_next.end(), completer));
        return true;
    }

    completer *completer::wildcard() {
        for (auto it = this->_next.begin(); it != this->_next.end(); it++)
            if (dynamic_cast<tab::wildcard *>(*it) != nullptr)
                return *it;
        completer *next = new tab::wildcard(this);
        this->_next.push_back(next);
        return next;
    }

    bool completer::unregisterWildcard() {
        for (auto it = this->_next.begin(); it != this->_next.end(); it++)
            if (dynamic_cast<tab::wildcard *>(*it) != nullptr) {
                this->_next.erase(std::find(this->_next.begin(), this->_next.end(), *it));
                (*it)->root = nullptr;
                delete *it; //Automaticly get removed
                return true;
            }
        return false;
    }

    std::vector<completer *> CompleterBase::availabilities(std::vector<std::string> &args/*, int index*/) {
        vector<completer *> current(this->_next);

        for (int i = 0; i < args.size() - 1; i++) {
            vector<completer *> out;
            string str = args.size() > i ? args[i] : "";
            for (auto elm : current) {
                if (elm->acceptExact(str)) {
                    for (auto e2 : elm->next())
                        out.push_back(e2);
                }
            }
            current = out;
        }

        vector<completer *> out;
        for (auto it = current.begin(); it != current.end(); it++) {
            if ((*it)->accept(args[args.size() - 1]) && (*it)->expected != "") {
                out.push_back(*it);
            }
        }

        return out;
    }

    wildcard::wildcard(completer *parent) : completer(parent, "", true) {}

    bool wildcard::accept(std::string in) {
        return true;
    }

    bool wildcard::acceptExact(std::string in) {
        return true;
    }
}