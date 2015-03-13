#include "globals.h"
#include "shadow.h"
#include "counters.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

map<IDNoType,u64> instrCounts;
map<IDNoType,u64> callCounts;
extern Symbols symTable;
extern bool ShowUnknown;

void PrintInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    cout << setw(30) << "Function Name" << setw(12) << "# Instr." << endl;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++ )
    {
        IDNoType fid =  iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            u64 instrs = iter->second;
            cout << setw(30) << symTable.GetSymName(fid) << setw(10) << instrs << endl;
        }
    }
    cout << setw(30) << "Total" << setw(10) << TotalInstrCount() << endl;
}

void PrintInstrPercents()
{
    map<IDNoType,u64>::iterator iter;
    cout << setw(30) << "Function Name" << setw(12) << "% Instr." << endl;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++ )
    {
        IDNoType fid =  iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            cout << setw(30) << symTable.GetSymName(fid) << setw(10) << GetInstrCountPercent(fid) << endl;
        }
    }
}

u64 TotalInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    u64 total = 0;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
        IDNoType fid = iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
            total += (iter->second);
    }
    return total;
}

u64 MaxInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    u64 max = 0;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
        IDNoType fid = iter->first;
        if(!ShowUnknown && fid==UnknownID)
            continue;
        else
        {
            if( max < (iter->second) )
                max = iter->second;
        }
    }
    return max;
}

float GetInstrCountPercent(IDNoType fid)
{
    u64 count = instrCounts[fid];
    return floor( 100.0 * count / TotalInstrCount() );
}


u64 GetCallCount(IDNoType fid)
{
    //TODO what if the entry for an fid does not exist?
    return callCounts[fid];
}
