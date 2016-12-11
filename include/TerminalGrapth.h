//
// Created by wolverindev on 04.12.16.
//

#pragma once

#include <vector>
#include <string>
#include "CString.h"

namespace Terminal {
        struct ValueTableEntry {
            double x;
            double y;

            inline bool operator==(const ValueTableEntry);
        };

        class ValueTable {
            public:
                void getValue(double x, ValueTableEntry* out);
                void addValue(ValueTableEntry value);
                void removeValue(double x);
                std::vector<ValueTableEntry> getValues();

                CChar dchar;
                bool gussUnknownValue = false;
            private:
                std::vector<ValueTableEntry> points;
        };

        class Grapth {
            public:
                Grapth();

                std::vector<std::string> buildLine(int xSize, int ySize, int xScaleRows);

                int startX;
                int endX;
                int stepX;

                int startY;
                int endY;

                CString yAxisName;
                CString xAxisName;


                std::vector<ValueTable> tables;

            private:
                int buildYScale(CString *lines, int size);
                void buildXScale(CString* lines,int lSize,CString prefix,int size);

                void buildGraph(int startIndex, CString* linex, int lSize, double deltaPerLine, int, bool);

                int calculateXSection(int sizeX);
        };
}