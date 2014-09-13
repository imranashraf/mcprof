#include "globals.h"
#include "shadow.h"
#include "engine4.h"
#include "commatrix.h"
#include "symbols.h"

extern CallStackType CallStack; // main call stack
extern std::map <std::string,IDNoType> FuncName2ID;
extern Symbols symTable;

// all calls written to this file
static string perCallFileName("percallaccesses.out");

// all calls to a single func, index is call no
typedef vector<Call> AllCalls2OneFtnType;

// all calls to all functions, index is the funcID
typedef map<u16,AllCalls2OneFtnType> AllCalls2AllFtnsType;

AllCalls2AllFtnsType AllCalls;

Call* currCall; // pointer to current call
/*
whenever a func is entered, a new call is created.
This pointer then points to that new call, which is
update only at function enty. Hence
it is not required to determine the right Call
on each record read/write access.
*/

// This is used to assign sequence numbers to all calls globaly. Based on this
// seq no, temporal orders of calls can be obtained
static u64 GlobalCallSeqNo=0;

// TODO optimize fname to funcid
void SetCurrCall(string& fname)
{
    IDNoType funcid = FuncName2ID[fname];
    D2ECHO("Setting currCall for " << FUNC(funcid) );

    auto it = AllCalls.find(funcid);
    if( it == AllCalls.end() )
    {
        AllCalls[funcid] = AllCalls2OneFtnType();
    }

    Call newCall;
    AllCalls[funcid].push_back(newCall); // add a new call
    currCall = &( AllCalls[funcid].back() );

    //set callpath of currCall by traversing call stack
    currCall->CallPath = CallStack;
    currCall->SeqNo = GlobalCallSeqNo++;
}

void RecordWriteEngine4(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();

    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    IDNoType objid = GetObjectID(addr);
    D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

    // TODO check weather we need to some thing special for unknown objects
//     if(objid != UnknownID)
//     {
//     }

    currCall->Writes[objid]+=size;
    for(u32 i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadEngine4(uptr addr, u32 size)
{
//     IDNoType cons = CallStack.Top(); // TODO remove it if not needed
//     D2ECHO("Recording Read " << VAR(size) << FUNC(cons) << ADDR(addr) << dec);
    D2ECHO("Recording Read " << VAR(size) << " at " << ADDR(addr) << dec);

    IDNoType objid = GetObjectID(addr);
    D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );
    currCall->Reads[objid]+=size;

    //     IDNoType prod;
    //TODO do we need to record the prod to obj, and obj to cons
    // as separate entities ?
}

// print a single call to a single function
void PrintCall(Call& call, ofstream& fout)
{
    fout << "Call Seq No : " << call.SeqNo << "\n" ;
    call.CallPath.Print(fout);
    for ( auto& readPair : call.Reads)
    {
        IDNoType oid = readPair.first;
        fout << "Reads from " << symTable.GetSymName(oid) << " : " << readPair.second << "\n" ;
    }

    for ( auto& writePair : call.Writes)
    {
        IDNoType oid = writePair.first;
        fout << "Writes to " << symTable.GetSymName(oid) << " : " << writePair.second << "\n" ;
    }
}

// print all calls to a single function
void PrintCalls(AllCalls2OneFtnType& calls, ofstream& fout)
{
    u32 totalCalls = calls.size();
    fout <<"Total Calls : " << totalCalls << "\n";
    u32 cno=0;
    for ( auto& call : calls)
    {
        fout << "Call No : " << cno << "\n";
        PrintCall(call, fout);
        cno++;
    }
}

// print all call to all functions
void PrintAllCalls()
{
    ofstream fout;
    OpenOutFile(perCallFileName, fout);

    fout << "Printing All Calls\n";
    for ( auto& callpair :AllCalls)
    {
        IDNoType fid = callpair.first;
        fout << "Printing Calls to " << symTable.GetSymName(fid) << "\n";
        PrintCalls(callpair.second, fout);
    }

    fout.close();
}
