#pragma once

#include "Terminal.h"
#include <string>
#include <vector>

namespace terminal {
    namespace tab {
        class completer {
            friend class CompleterBase;
        public:
            completer(std::string word, bool ignoreCase = true);
            completer(completer* before,std::string word, bool ignoreCase = true);
            ~completer();

            completer* parameter(std::string parameter);
            bool unregister(std::string parameter);

            bool registerTabCompleter(completer* completer);
            bool unregisterTabCompleter(completer* completer);

            completer* wildcard();
            bool unregisterWildcard();

            virtual bool accept(std::string in);
            virtual bool acceptExact(std::string in);

            std::vector<completer*> next();
            std::string expected;
        private:
            completer* root = nullptr;
            bool ignoreCase = true;
            std::vector<completer*> _next;
        };

        class wildcard : public completer {
        public:
            wildcard(completer*);

            virtual bool accept(std::string in) override;
            virtual bool acceptExact(std::string in) override;
        };

        class CompleterBase : public completer {
        public:
            CompleterBase();
            std::vector<completer*> availabilities(std::vector<std::string> &args);

            TabCompleter* getBasedCompleter(){ return &this->baseCompleter; }
        private:
            TabCompleter baseCompleter;
        };
    }
}

#ifdef SUPPORT_LEGACY
    namespace Termianl = terminal::tab;
#endif