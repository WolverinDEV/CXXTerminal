//
// Created by wolverindev on 04.12.16.
//

#include <malloc.h>
#include <algorithm>
#include "../../include/TerminalGrapth.h"
#include "../../include/QuickTerminal.h"
#include <cmath>

using namespace std;
using namespace Terminal;

void Terminal::ValueTable::addValue(ValueTableEntry value) {
    removeValue(value.x); //Remove old points
    points.push_back(value);

    sort(this->points.begin(), this->points.end(), [](ValueTableEntry& a,ValueTableEntry& b){
        return a.x < b.x;
    });
}

void Terminal::ValueTable::removeValue(double x) {
    vector<ValueTableEntry> removing;
    for(auto it = this->points.begin(); it != this->points.end(); it++)
        if(it->x == x)
            removing.push_back(*it);

    for(auto it = removing.begin(); it != removing.end(); it++)
        this->points.erase(find(this->points.begin(), this->points.end(), *it));
}
std::vector<ValueTableEntry> Terminal::ValueTable::getValues() {
    return this->points;
}

ValueTableEntry* getPointHigher(vector<ValueTableEntry>& list, double x){
    for(auto it = list.begin(); it != list.end(); it++){
        if(it.operator*().x >= x)
            return &(*it);
    }
    return nullptr;
}
ValueTableEntry* getPointLower(vector<ValueTableEntry>& list, double x){
    ValueTableEntry* current = nullptr;

    for(auto it = list.begin(); it != list.end(); it++){
        if(it.operator*().x > x)
            return current;
        else
            current = &(*it);
    }
    return current;
}

void Terminal::ValueTable::getValue(double x, ValueTableEntry* out) {
    if(this->points.empty()){
        *out = ValueTableEntry{x, 0};
        return;
    }
    if(this->points.size() == 1){
        *out = this->points[0];
        return;
    }

    ValueTableEntry* lower = getPointLower(this->points, x);

    ValueTableEntry* higher = getPointHigher(this->points, x);

    if(lower == higher && lower != nullptr){
        *out = ValueTableEntry{x, lower->y};
        return;
    }

    if(lower == nullptr){ //Get last pitch
        if(!gussUnknownValue){
            return;
        }
        lower = higher;
        higher = getPointHigher(this->points, lower->x+0.000000001);
    }

    if(higher == nullptr){
        if(!gussUnknownValue){
            return;
        }
        higher = lower;
        lower = getPointLower(this->points, higher->x-0.000000001);
    }

    //writeMessage("Use point ("+to_string(lower->x)+"/"+to_string(lower->y)+") as lower and ("+to_string(higher->x)+"/"+to_string(higher->y)+") as higher");
    double yPitch = higher->y - lower->y;
    double xPitch = higher->x - lower->x;
    double pitch = yPitch / xPitch;
    //writeMessage("Pitch: "+to_string(pitch));
    double xDiff = x - lower->x;
    //writeMessage("Out: "+to_string(lower->y + xDiff * pitch)+" for "+to_string(x));
    *out = ValueTableEntry{x, lower->y + xDiff * pitch};
}


bool Terminal::ValueTableEntry::operator==(const ValueTableEntry val) {
    return x == val.x && y == val.y;
}

inline string leftPad(string in, int size){
    string out = in;
    while(out.size() < size)
        out = " "+out;
    return out;
}

inline string toString(double num, int digsits){
    char buffer[100];
    writeMessage("To string: "+to_string(num));
    sprintf(buffer, ("%."+to_string(digsits)+"f").c_str(), num);
    return string(buffer);
}

inline int getSize(double number, int digsits){
    return toString(number, digsits).size();
}

Grapth::Grapth() {
    this->xAxisName = CString("X");
    this->yAxisName = CString("Y");
}

int Terminal::Grapth::calculateXSection(int sizeX){
    double stepsPerColumn = (endX-startX) / stepX;
    return (int) floor(sizeX / stepsPerColumn);
}

void Terminal::Grapth::buildGraph(int startIndex, CString *linex, int lSize, double deltaPerLine, int xSize, bool smoth) {
    for(int i = 0;i<lSize;i++)
        while (linex[i].chars.size() < startIndex+xSize)
            linex[i] += "§r ";
    double stepsPerColumn = (float) (endX-startX) / (float) xSize;
    writeMessage("Delta: "+to_string(deltaPerLine)+" - "+to_string(endY-startY)+" - "+to_string(lSize));
    writeMessage("Steps: "+to_string(stepsPerColumn)+" - "+to_string(xSize));
    double count = startX;

    ValueTableEntry* entry = new ValueTableEntry;
    for(int i = 1;i<xSize;i++){
        if(i > 1) { //Dont calculate the first
            for (auto graph = this->tables.begin(); graph != this->tables.end(); graph++) {
                graph->getValue((double) count, entry);
                double y = entry->y;
                if (0) { //TODO add graph fill up

                } else {
                    if (y >= startY-deltaPerLine && y <= endY+deltaPerLine) {
                        for (int line = 0; line < lSize; line++) {
                            if (y > startY + deltaPerLine * (line - 1) && startY + deltaPerLine * (line + 1) >= y) {
                                linex[lSize - line - 1].chars[startIndex + (i - 1)] = graph->dchar;
                                break;
                            }
                        }
                    }
                }
            }
        }
        count += stepsPerColumn;
    }
    delete entry;
}

void Terminal::Grapth::buildXScale(CString *lines, int lSize, CString prefix, int sizeX) {
    for(int i = 0;i<lSize;i++)
        lines[i] += prefix;
    int lastUsed = 0;
    double stepsPerColumn = (float) (endX-startX) / (float) sizeX;
    double count = startX;
    int sectionCount = calculateXSection(sizeX);

    for(int i = 0;i<sizeX;i++){
        bool number = i % sectionCount == 0 && i != 0;
        lines[0] += number || i == 0 ? "+" : "-";
        if(number){
            int step = 1;
            while (1){
                if(step >= lSize)
                    goto afterNumber;
                CString& str = lines[step++];
                if(str.chars.size() < i+prefix.chars.size()){
                    step--;
                    break;
                }
            }
            while(lines[step].chars.size() < i+prefix.chars.size())
                lines[step] += " ";
            lines[step] += toString(count, 2)+ " ";
            if(step > lastUsed)
                lastUsed = step;
        }
        goto afterNumber; //useless?
        afterNumber:
        {
            count += stepsPerColumn;
        };
    }
    if(lastUsed+1<lSize)
        lastUsed+=1;
    CString str = lines[lastUsed];
    while (str.chars.size()+xAxisName.chars.size() < sizeX + prefix.chars.size())
        str += " ";
    if(str.chars.size()+xAxisName.chars.size() >= sizeX + prefix.chars.size()){
        //TODO cut
    }
    lines[lastUsed] = str; //useless?
    lines[lastUsed] += xAxisName;
}

int Terminal::Grapth::buildYScale(CString *lines, int size){
    double delta = (double) (endY-startY) / (double) (size-1);

    int digSize = max(getSize(startY+delta*size, 2),(int) yAxisName.chars.size());
    for(int i = 0;i<size;i++){
        if(i == 0){
            lines[i] = " "+leftPad(yAxisName.str(), digSize)+ "  ";
            continue;
        }
        lines[i] += " "+leftPad(toString(endY-delta*(i-1), 2), digSize)+" |";
    }
    return digSize + 2;
}

std::vector<std::string> Terminal::Grapth::buildLine(int xSize, int ySize, int xScaleRows) {
    CString* lines = new CString[ySize];
    for(int i = 0;i<ySize;i++){
        lines[i] = CString("");
        //for(int y = 0;y<xSize;y++)
        //    lines[i] += " ";
    }
    int prefixLength = buildYScale(lines, ySize-xScaleRows);

    CString str;
    for(int i = 0;i<prefixLength;i++)
        str += " ";

    buildXScale((CString*) (lines+ySize-xScaleRows), xScaleRows, str, xSize);

    buildGraph(prefixLength, lines + 1, ySize-xScaleRows,(endY-startY)/(double)(ySize-xScaleRows-1), xSize, false);

    vector<string> out;
    for(int i = 0;i<ySize;i++)
        out.push_back(lines[i].str());
    return out;
}


int cspline_interpolate(ValueTableEntry *tab, int ntab, ValueTableEntry *data_out, int nout)
{
    int i, k, jo, ilo, ihi, im;
    double sigma, Un, p, Qn, aux, a, b;
    static int nwork;
    static double *Y2, *U;

    // need 3 points minimum
    if (ntab <= 3)
        return 1;

    if (nwork < ntab)
    {
        if (Y2)
        {
            Y2 = (double*)realloc(Y2, sizeof(*Y2) * ntab);
            U = (double*)realloc(U, sizeof(*U) * ntab);
        }
        else
        {
            Y2 = (double*)malloc(sizeof(*Y2) * ntab);
            U = (double*)malloc(sizeof(*U) * ntab);
        }
        nwork = ntab;
    }

    // controlling the lower boundary condition
    Y2[0] = U[0] = 0.0;

    // decomposition loop of the triangular algorithm
    for (i = 1; i < ntab - 1; i++)
    {
        sigma = (tab[i].x - tab[i - 1].x) / (tab[i + 1].x - tab[i - 1].x);
        p = sigma * Y2[i - 1] + 2.0;
        Y2[i] = (sigma - 1.0) / p;
        U[i] = ((tab[i + 1].y - tab[i].y) / (tab[i + 1].x - tab[i].x)) -
               ((tab[i].y - tab[i - 1].y) / (tab[i].x - tab[i - 1].x));
        U[i] = (6.0 * U[i] / (tab[i + 1].x - tab[i - 1].x) - sigma * U[i - 1]) / p;
    }

    // controlling the upper boundary condition
    Qn = Un = 0.0;
    Y2[ntab - 1] = (Un - Qn * U[ntab - 2]) / (Qn * Y2[ntab - 2] + 1.0);

    // backsubstitution loop of the tridiagonal algorithm
    for (k = ntab - 2; k >= 0; k--)
        Y2[k] = Y2[k] * Y2[k + 1] + U[k];

    // setup
    double xrange = tab[ntab - 1].x - tab[0].x;
    double stepsize = xrange / (nout - 1);

    data_out[0].x = tab[0].x;
    data_out[0].y = tab[0].y;

    /* constructing the cubic splines */
    for (jo = 0, i = 1; i < nout; i++)
    {
        data_out[i].x = data_out[0].x + (i * stepsize);

        /* center */
        ilo = jo;   // lo
        ihi = ntab;  // hi
        while ((ihi - ilo) > 1)
        {
            im = (ihi + ilo) / 2;
            if (data_out[i].x > tab[im].x)
                ilo = im;
            else
                ihi = im;
        }

        jo = ilo;

        /* interpolate from center outward */
        aux = tab[ihi].x - tab[ilo].x;
        a = (tab[ihi].x - data_out[i].x) / aux;
        b = (data_out[i].x - tab[ilo].x) / aux;

        data_out[i].y = (a * tab[ilo].y + b * tab[ihi].y +
                         ((a * a * a - a) * Y2[ilo] + (b * b * b - b) * Y2[ihi]) *
                         (aux * aux) / 6.0);
    }

    data_out[nout - 1] = tab[ntab - 1];
    return 0; // no error
}

// used to reverse or restore data order for data with descending X
void spline_flipX(ValueTableEntry* data_in, int ndata)
{
    int start = 0;
    int end = ndata -1;
    ValueTableEntry tmp;
    while(start < end)
    {
        tmp = data_in[start];
        data_in[start] = data_in[end];
        data_in[end] = tmp;
        start++;
        end--;
    }
}