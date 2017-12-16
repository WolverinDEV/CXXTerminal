//
// Created by wolverindev on 11.12.16.
//

#pragma once

#include <vector>
#include <string>
#include "CString.h"

namespace Terminal {
    namespace Graphics {
        namespace Table {
            struct ColumnValue {
                TableColumn column;
                std::string value;
            };

            struct TableEntry {
                ColumnValue* columns;
                int ncolumns;
            };

            struct TableColumn {
                int maxSize;
                std::string name;
            };

            class Table {
                public:
                    void addEntry(TableEntry entry);
                    void removeEntry(TableEntry entry);
                    std::vector<TableEntry> getEntries();

                    std::vector<TableColumn> getColumns();
                    void addColumn(TableColumn row);
                    void removeColumn(TableColumn row);

                    std::vector<std::string> buildTable();
                private:
                    std::vector<TableEntry> entries;
                    std::vector<TableColumn> columns;
            };
        }
    }
}