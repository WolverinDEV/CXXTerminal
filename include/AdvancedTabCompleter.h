//
// Created by wolverindev on 22.11.16.
//

#include "Terminal.h"
#include <string>
#include <vector>

namespace Terminal {
        class DependCompleter {
                friend class AvTabCompleter;
            public:
                DependCompleter(std::string, bool ignoreCase = true);
                DependCompleter(DependCompleter*,std::string, bool ignoreCase = true);
                ~DependCompleter();

                DependCompleter* parameter(std::string parameter);
                bool unregister(std::string parameter);

                bool registerTabCompleter(DependCompleter* completer);
                bool unregisterTabCompleter(DependCompleter* completer);

                DependCompleter* wildcard();
                bool unregisterWildcard();

                virtual bool accept(std::string in);
                virtual bool acceptExact(std::string in);

                std::vector<DependCompleter*> getNext();
                std::string expected;
            private:
                DependCompleter* root;
                std::vector<DependCompleter*> next;

                bool ignoreCase;
        };
        class WildcardCompleter : public DependCompleter {
            public:
                WildcardCompleter(DependCompleter*);

                virtual bool accept(std::string in) override;

                virtual bool acceptExact(std::string in) override;
        };

        class AvTabCompleter : public DependCompleter{
            public:
                AvTabCompleter();
                std::vector<DependCompleter*> getAvaribilities(std::vector<std::string>& args/*, int index*/);

                TabCompleter* getBasedCompleter(){
                    return &this->baseCompleter;
                }
            private:
                TabCompleter baseCompleter = NULL;

                void pushbackArg(int index, std::string& str);
        };
}