#include "globals.h"
#include "shadow.h"
#include "instrcount.h"
#include "commatrix.h"
#include "symbols.h"
#include "callstack.h"

map<IDNoType,u64> instrCounts;
extern Symbols symTable;

void PrintInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    cout << setw(30) << "Function Name" << setw(12) << "# Instr." << endl;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
        IDNoType fid =  iter->first;
        u64 instrs = iter->second;
        cout << setw(30) << symTable.GetSymName(fid) << setw(10) << instrs << endl;
    }
    cout << setw(30) << "Total" << setw(10) << TotalInstrCount() << endl;
}

void PrintInstrPercents()
{
    map<IDNoType,u64>::iterator iter;
    cout << setw(30) << "Function Name" << setw(12) << "% Instr." << endl;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
        IDNoType fid =  iter->first;
        cout << setw(30) << symTable.GetSymName(fid) << setw(10) << GetInstrCountPercent(fid) << endl;
    }
}

u64 TotalInstrCount()
{
    map<IDNoType,u64>::iterator iter;
    u64 total = 0;
    for( iter = instrCounts.begin(); iter != instrCounts.end(); iter++)
    {
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
        if( max < (iter->second) )
            max = iter->second;
    }
    return max;
}

float GetInstrCountPercent(IDNoType fid)
{
    u64 count = instrCounts[fid];
    return floor( 100.0 * count / TotalInstrCount() );
}