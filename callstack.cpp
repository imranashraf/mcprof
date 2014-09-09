#include "globals.h"
#include "shadow.h"
#include "callstack.h"
#include "commatrix.h"

extern std::map <std::string,u16> FuncName2ID;

extern Symbols symTable;

void CallStackType::Print(ofstream& fout)
{
    string stackftns("");
    if( !stack.empty()) stackftns += symTable.GetSymName(stack[0]);
    for ( u16 ftn=1; ftn < stack.size(); ftn++)
        stackftns += " -> " + symTable.GetSymName(stack[ftn]);
    fout << "Call Stack : " << stackftns << "\n";
    D1ECHO("Call Stack : " << stackftns);
}
