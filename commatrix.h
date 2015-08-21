/*
         __       __   ______   _______   _______    ______   ________
        /  \     /  | /      \ /       \ /       \  /      \ /        |
        $$  \   /$$ |/$$$$$$  |$$$$$$$  |$$$$$$$  |/$$$$$$  |$$$$$$$$/
        $$$  \ /$$$ |$$ |  $$/ $$ |__$$ |$$ |__$$ |$$ |  $$ |$$ |__
        $$$$  /$$$$ |$$ |      $$    $$/ $$    $$< $$ |  $$ |$$    |
        $$ $$ $$/$$ |$$ |   __ $$$$$$$/  $$$$$$$  |$$ |  $$ |$$$$$/
        $$ |$$$/ $$ |$$ \__/  |$$ |      $$ |  $$ |$$ \__$$ |$$ |
        $$ | $/  $$ |$$    $$/ $$ |      $$ |  $$ |$$    $$/ $$ |
        $$/      $$/  $$$$$$/  $$/       $$/   $$/  $$$$$$/  $$/

                A Memory and Communication Profiler

 * This file is a part of MCPROF.
 * https://bitbucket.org/imranashraf/mcprof
 * 
 * Copyright (c) 2014-2015 TU Delft, The Netherlands.
 * All rights reserved.
 * 
 * MCPROF is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCPROF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MCPROF.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Authors: Imran Ashraf
 *
 */

#ifndef COMMATRIX_H
#define COMMATRIX_H

#include "globals.h"
#include "symbols.h"

#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <cmath>
#include <iomanip>

extern Symbols symTable;

using namespace std;

class Matrix2D
{
private:
    vector< vector<float> > Matrix;
    //most of the applications have functions less than 256
    static const u32 DEFAULT_SIZE = 9000; //15000
    set<IDNoType> FilledRows;
    set<IDNoType> FilledCols;

public:
    Matrix2D();
    Matrix2D(IDNoType size);
    void inline RecordCommunication(IDNoType prod, IDNoType cons, u32 size)
    {
        D2ECHO("Recording Communication b/w " << FUNC(prod) << " and "
            << FUNC(cons) << " of size: " << size );

        if( prod < Matrix.size() && cons < Matrix.size() )
            Matrix[prod][cons] += size;
    }

    float MaxCommunication(u16 StartID);
    void Print(ostream &fout);
    void PrintMatrix(ostream &fout);
    void PrintDot(ostream &dotout);
    void UpdateEmptyRowsCols(IDNoType StartID, IDNoType EndID);
    bool IsFilledRow(IDNoType r);
    bool IsFilledCol(IDNoType c);
    void CheckLoopIndependence( IDNoType loopNo, u32 nIterations);
};

#endif
