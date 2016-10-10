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

#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include "globals.h"

#include <iostream>
#include <map>

using namespace std;
class SparseMatrix
{
private:
    std::map<std::pair<int, int>, float> matrix;
public:
    SparseMatrix(){}
    void Insert(int r, int c, float val)
    {
        std::pair<int, int> cord = make_pair(r,c);
        matrix[cord] = val;
    }
    void inline RecordCommunication(int r, int c, float val)
    {
        std::pair<int, int> cord = make_pair(r,c);
        auto it = matrix.find(cord);
        if( it != matrix.end() )
        {
            it->second = it->second + val;
        }
        else
        {
            matrix[cord] = val;
        }
    }
    void Clear()
    {
        matrix.clear();
    }
    bool CheckLoopIndependence(u32 nIterations)
    {
        bool result=true;
        for (IDNoType pid=1; pid<nIterations-1; ++pid) // iterations start from 1
        {
            for (IDNoType cid=pid+1; cid<nIterations; ++cid)
            {
                std::pair<int, int> cord = make_pair(pid,cid);
                auto it = matrix.find(cord);
                if( it != matrix.end() )
                {
                    if( it->second > 0 )
                    {
                        result = false;
                        break;
                    }
                }
            }
        }
        return result;
    }
    void PrintMatrix(int maxDim)
    {
        #define ALIGNMENT setw(8)
        std::ofstream mout;
        string matrixFileName("loopdependences.dat");
        OpenOutFile(matrixFileName, mout);
        D2ECHO("Printing loop dependences matrix as table in " << matrixFileName);

        mout << ALIGNMENT << "#";
        for (IDNoType c=1; c<maxDim; c++)
        {
            mout << ALIGNMENT << c;
        }
        mout << endl;

        for(int r=1; r<maxDim; r++)
        {
            mout << ALIGNMENT << r;
            for(int c=1; c<maxDim; c++)
            {
                std::pair<int, int> cord = make_pair(r,c);
                auto it = matrix.find(cord);
                if( it != matrix.end() )
                {
                    mout << ALIGNMENT << it->second;
                }
                else
                {
                    mout << ALIGNMENT << 0;
                }
            }
            mout << endl;
        }
        mout.close();
        #undef ALIGNMENT
    }
    void PrintMatrixAsSparse(int maxDim)
    {
        std::ofstream mout;
        string matrixFileName("loopdependences.dat");
        OpenOutFile(matrixFileName, mout);
        D2ECHO("Printing loop dependences matrix as sparse representation in " << matrixFileName);

        for(int r=1; r<maxDim; r++)
        {
            for(int c=1; c<maxDim; c++)
            {
                std::pair<int, int> cord = make_pair(r,c);
                auto it = matrix.find(cord);
                if( it != matrix.end() )
                {
                    mout << r << " " << c << " " << it->second << endl;
                }
            }
        }
        mout.close();
        #undef ALIGNMENT
    }
};


#endif
