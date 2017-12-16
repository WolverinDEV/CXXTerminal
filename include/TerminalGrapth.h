//
// Created by wolverindev on 04.12.16.
//

#pragma once

#include <vector>
#include <string>
#include "CString.h"

namespace Terminal {
    namespace Graphics {
        namespace Diagram {
            struct Point {
                double x;
                double y;

                inline bool operator==(const Point);
            };

            class Graph {
                public:
                    void getValue(double x, Point* out);
                    void addValue(Point value);
                    void removeValue(double x);
                    std::vector<Point> getValues();

                    CChar dchar;
                    bool gussUnknownValue = false;
                private:
                    std::vector<Point> points;
            };

            class CoordinateSystem {
                public:
                    CoordinateSystem();

                    std::vector<std::string> buildLine(int xSize, int ySize, int xScaleRows);

                    int startX;
                    int endX;
                    int stepX;

                    int startY;
                    int endY;

                    CString yAxisName;
                    CString xAxisName;


                    std::vector<Graph> tables;

                private:
                    int buildYScale(CString *lines, int size);
                    void buildXScale(CString* lines,int lSize,CString prefix,int size);

                    void buildGraph(int startIndex, CString* linex, int lSize, double deltaPerLine, int, bool);

                    int calculateXSection(int sizeX);
            };
        }
    }
}