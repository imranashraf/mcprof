#include "globals.h"
#include "shadow.h"
#include "callstack.h"
#include "commatrix.h"

extern std::map <std::string,u16> Name2ID;
extern std::map <u16,std::string> ID2Name;

void CallStackType::Print(ofstream& fout)
{
    string stackftns("");
    if( !stack.empty()) stackftns += ID2Name[ stack[0] ];
    for ( u16 ftn=1; ftn < stack.size(); ftn++)
        stackftns += " -> " + ID2Name[ stack[ftn] ] ;
//         stackftns += " -> " + ID2Name[ stack[ftn] ] ;
    fout << "Call Stack : " << stackftns << "\n";
    ECHO("Call Stack : " << stackftns);
}
