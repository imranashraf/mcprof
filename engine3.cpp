#include "globals.h"
#include "shadow.h"
#include "engine3.h"
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
Call tempCall;  // To avoid seg fault for info stored b4 calling main
Call* currCall=&tempCall; // pointer to current call
/*
whenever a func is entered, a new call is created. "currCall" pointer then
points to this new call, which is updated only at function enty/exit. Hence it
is not required to determine the right Call per read/write access.
*/

/*
GlobalCallSeqNo is used to assign sequence numbers to all calls globaly.
Based on this seq no, temporal orders of calls can be obtained
*/
static u64 GlobalCallSeqNo=0;

void SetCurrCall()
{
    IDNoType funcid = CallStack.Top();

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

void SetCurrCallOnRet()
{
    IDNoType funcid = CallStack.Top();
    if(funcid != UnknownID)
    {
        currCall = &( AllCalls[funcid].back() );
    }
}

void RecordWriteEngine3(uptr addr, u32 size)
{
    IDNoType prod = CallStack.Top();

    D2ECHO("Recording Write:  " << VAR(size) << FUNC(prod) << ADDR(addr));
    IDNoType objid = GetObjectID(addr);
    D2ECHO( ADDR(addr) << " " << symTable.GetSymName(objid) << "(" << objid << ")" );

    CHECK(currCall);
    currCall->Writes[objid]+=size;
    for(u32 i=0; i<size; i++)
    {
        SetProducer(prod, addr+i);
    }
}

void RecordReadEngine3(uptr addr, u32 size)
{
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
    //for ( auto& readPair : call.Reads)
    map<IDNoType,float>::iterator iter;
    for( iter = call.Reads.begin(); iter != call.Reads.end(); iter++)
    {
        IDNoType oid = iter->first;
        fout << "Reads from " << symTable.GetSymName(oid) << " : " << iter->second << "\n" ;
    }

    //for ( auto& writePair : call.Writes)
    //map<IDNoType,float>::iterator iter;
    for( iter = call.Writes.begin(); iter != call.Writes.end(); iter++)
    {
        IDNoType oid = iter->first;
        fout << "Writes to " << symTable.GetSymName(oid) << " : " << iter->second << "\n" ;
    }
}

// print all calls to a single function
void PrintCalls(AllCalls2OneFtnType& calls, ofstream& fout)
{
    u32 totalCalls = calls.size();
    fout <<"Total Calls : " << totalCalls << "\n";
    u32 cno=0;

    vector<Call>::iterator iter;
    //for ( auto& call : calls)
    for(iter=calls.begin(); iter!=calls.end(); iter++)
    {
        fout << "Call No : " << cno << "\n";
        PrintCall(*iter, fout);
        cno++;
    }
}

// print all call to all functions
void PrintAllCalls()
{
    ofstream fout;
    OpenOutFile(perCallFileName, fout);

    fout << "Printing All Calls\n";
    //for ( auto& callpair :AllCalls)
    AllCalls2AllFtnsType::iterator iter;
    for(iter=AllCalls.begin(); iter!=AllCalls.end(); iter++)
    {
        IDNoType fid = iter->first;
        fout << "Printing Calls to " << symTable.GetSymName(fid) << "\n";
        PrintCalls(iter->second, fout);
    }

    fout.close();
}
