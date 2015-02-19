
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include "markers.h"
#include "globals.h"
#include "symbols.h"
#include "pintrace.h"
#include "commatrix.h"
#include "shadow.h"
#include "callstack.h"
#include "engine1.h"
#include "engine2.h"
#include "engine3.h"
#include "instrcount.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <set>
#include <map>
#include <deque>
#include <algorithm>
#include <cstring> 

/* ================================================================== */
// Global variables
/* ================================================================== */
extern map <string,IDNoType> FuncName2ID;
extern Symbols symTable;
extern Matrix2D ComMatrix;

std::ofstream dotout;
std::ofstream mout;
std::ofstream pcout;

CallStackType CallStack;
CallSiteStackType CallSiteStack;

void (*WriteRecorder)(uptr, u32);
void (*ReadRecorder)(uptr, u32);
void (*InstrCounter)(ADDRINT);

bool TrackObjects;
bool RecordAllAllocations;
bool FlushCalls;
u32 FlushCallsLimit;
bool ShowUnknown;
bool TrackZones;
bool TrackStartStop;

extern map<IDNoType,u64> instrCounts;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobMatrixFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "MatFile", "matrix.out",
                            "specify file name for matrix output");

KNOB<string> KnobDotFile(KNOB_MODE_WRITEONCE,  "pintool",
                         "DotFile", "communication.dot",
                         "specify file name for output in dot");

KNOB<string> KnobPerCallFile(KNOB_MODE_WRITEONCE,  "pintool",
                         "PerCallFile", "percallaccesses.out",
                         "specify file name for per call output file");

KNOB<BOOL> KnobRecordStack(KNOB_MODE_WRITEONCE, "pintool",
                           "RecordStack","0", "Include Stack Accesses");

KNOB<BOOL> KnobTrackObjects(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackObjects", "0", "Track the objects");

KNOB<UINT32> KnobEngine(KNOB_MODE_WRITEONCE,  "pintool",
                        "Engine", "1",
                        "specify engine to be used");

KNOB<BOOL> KnobSelectFunctions(KNOB_MODE_WRITEONCE, "pintool",
                               "SelectFunctions", "0",
                               "Instrument only the selected functions. \
                                User provides functions in <SelectFunctions.txt> file");

KNOB<BOOL> KnobSelectObjects(KNOB_MODE_WRITEONCE, "pintool",
                             "SelectObjects", "0",
                             "Instrument only the selected objects. \
                              User provides objects in <SelectObjects.txt> file");

KNOB<BOOL> KnobMainExecutableOnly(KNOB_MODE_WRITEONCE, "pintool",
                                  "MainExecOnly","1",
                                  "Trace functions that are contained only in the\
                                  executable image");

KNOB<BOOL> KnobRecordAllAllocations(KNOB_MODE_WRITEONCE, "pintool",
                                  "RecordAllAllocations","0",
                                  "Record all allocation sizes of objects");

KNOB<BOOL> KnobFlushCalls(KNOB_MODE_WRITEONCE, "pintool",
                                  "FlushCalls","1",
                                  "Flush calls at intermediate intervals. \
                                  Calls to a function in Engine 3 are flushed to \
                                  output file when they become greater than a \
                                  certain LIMIT(for now 5000) ) ");

KNOB<UINT32> KnobFlushCallsLimit(KNOB_MODE_WRITEONCE,  "pintool",
                        "FlushCallsLimit", "5000",
                        "specify LIMIT to be used for flushing calls.");

KNOB<BOOL> KnobShowUnknown(KNOB_MODE_WRITEONCE, "pintool",
                            "ShowUnknown", "0", "Show Unknown function in the output graphs");

KNOB<BOOL> KnobTrackStartStop(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackStartStop", "0", "Track start/stop markers in \
                            the code to start/stop profiling \
                            instead of starting from main()");

KNOB<BOOL> KnobTrackZones(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackZones", "0", "Track zone markers to profile per zones");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
VOID Usage()
{
    ECHO( "Memory and Data-Communication PROFiler.");
    ECHO( KNOB_BASE::StringKnobSummary() << endl);
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */
// This function is called for each basic block
// Use the fast linkage for calls
VOID PIN_FAST_ANALYSIS_CALL doInstrCount(ADDRINT c)
{
    IDNoType fid = CallStack.Top();
    instrCounts[fid] += c;
}

VOID dummyRecorder(uptr a, u32 b){}
VOID dummyInstrCounter(ADDRINT c){}

void SelectAnalysisEngine()
{
    InstrCounter = doInstrCount;
    switch( KnobEngine.Value() )
    {
    case 1:
        ReadRecorder = RecordReadEngine1;
        WriteRecorder = RecordWriteEngine1;
        break;
    case 2:
        ReadRecorder = RecordReadEngine2;
        WriteRecorder = RecordWriteEngine2;
        break;
    case 3:
        ReadRecorder = RecordReadEngine3;
        WriteRecorder = RecordWriteEngine3;
        break;
    default:
        ECHO("Specify a valid Engine number to be used");
        Die();
        break;
    }
}

void SelectDummyAnalysisEngine()
{
    ReadRecorder = dummyRecorder;
    WriteRecorder = dummyRecorder;
    InstrCounter = dummyInstrCounter;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*
 * This filter is used for functions which can be pushed on the call stack.
 * This means that the functions in the filter will not be visible on
 * the output profile.
 */
BOOL ValidFtnName(string name)
{
    return
        !(
            name.c_str()[0]=='_' ||
            name.c_str()[0]=='?' ||
            !name.compare("atexit") ||
            (name.find("std::") != string::npos) ||
#ifdef WIN32
            !name.compare("GetPdbDll") ||
            !name.compare("DebuggerRuntime") ||
            !name.compare("failwithmessage") ||
            !name.compare("pre_c_init") ||
            !name.compare("pre_cpp_init") ||
            !name.compare("mainCRTStartup") ||
            !name.compare("NtCurrentTeb") ||
            !name.compare("check_managed_app") ||
            !name.compare("DebuggerKnownHandle") ||
            !name.compare("DebuggerProbe") ||
            !name.compare("failwithmessage") ||
            !name.compare("unnamedImageEntryPoint")
#else
            !name.compare(".plt") ||
            !name.compare("_start") ||
            !name.compare("_init") ||
            !name.compare("_fini") ||
            !name.compare("__do_global_dtors_aux") ||
            !name.compare("__libc_csu_init") ||
            !name.compare("__gmon_start__") ||
            !name.compare("__libc_csu_fini") ||
            !name.compare("call_gmon_start") ||
            !name.compare("register_tm_clones") ||
            !name.compare("deregister_tm_clones") ||
            !name.compare("frame_dummy")
#endif
        );
}

/*
 * This is the filter used to filter function calls only. The main difference with
 * the above filter is that the calls to std::<> routines and calls to .plt
 * should also not be filtered so that the last call-site is properly recorded
 */
BOOL ValidFtnCallName(string name)
{
    return
        !(
            name.c_str()[0]=='_' ||
            name.c_str()[0]=='?' ||
            !name.compare("atexit") ||
#ifdef WIN32
            !name.compare("GetPdbDll") ||
            !name.compare("DebuggerRuntime") ||
            !name.compare("failwithmessage") ||
            !name.compare("pre_c_init") ||
            !name.compare("pre_cpp_init") ||
            !name.compare("mainCRTStartup") ||
            !name.compare("NtCurrentTeb") ||
            !name.compare("check_managed_app") ||
            !name.compare("DebuggerKnownHandle") ||
            !name.compare("DebuggerProbe") ||
            !name.compare("failwithmessage") ||
            !name.compare("unnamedImageEntryPoint")
#else
            !name.compare("_start") ||
            !name.compare("_init") ||
            !name.compare("_fini") ||
            !name.compare("__do_global_dtors_aux") ||
            !name.compare("__libc_csu_init") ||
            !name.compare("__gmon_start__") ||
            !name.compare("__libc_csu_fini") ||
            !name.compare("call_gmon_start") ||
            !name.compare("register_tm_clones") ||
            !name.compare("deregister_tm_clones") ||
            !name.compare("frame_dummy")
#endif
        );
}

// These are some global variables set by memory allocation functions
IDNoType currID;
u32 lastCallLocIndex=0;
u32 currSize;
uptr currStartAddress;

VOID RecordRoutineEntry(CHAR* rname)
{
    D1ECHO ("Entering Routine : " << rname );
    // enable tracing memory R/W in engines only after main

    CallStack.Push(FuncName2ID[rname]);
    CallSiteStack.Push(lastCallLocIndex);   // record the call site loc index
    #if (DEBUG>0)
    CallStack.Print();
    CallSiteStack.Print();
    #endif

    // In engine 3, to save time, the curr call is selected only at
    // func entry/exit, so that it does not need to be determined on each access
    if (KnobEngine.Value() == 3)
    {
        D1ECHO ("Setting Current Call for : " << rname );
        SetCurrCallOnEntry();
    }

    D1ECHO ("Entering Routine : " << rname << " Done" );
}

VOID RecordZoneEntry(INT32 zoneNo)
{
    IDNoType fid = CallStack.Top();
    string zoneName = symTable.GetSymName(fid) + to_string((long long)zoneNo);
    D1ECHO("Entring zone " << zoneName );

    // enter the zone in the symbole table if seeing for first time.
    // This cannot be done at instrumentation time as we dont know 
    // what kind of magic instruction it is at that time, we know that
    // only at analysis time
    if( !symTable.IsSeenFunctionName(zoneName) )
    {
        symTable.InsertFunction(zoneName);
    }

    // push it on to stack so that communication is associated with it
    CallStack.Push(FuncName2ID[zoneName]);

    CallSiteStack.Push(lastCallLocIndex);   // record the call site loc index

    #if (DEBUG>0)
    CallStack.Print();
    CallSiteStack.Print();
    #endif

    // In engine 3, to save time, the curr call is selected only at
    // func entry/exit, so that it does not need to be determined on each access
    if (KnobEngine.Value() == 3)
    {
        D1ECHO ("Setting Current Call for : " << zoneName );
        SetCurrCallOnEntry();
    }

    D1ECHO("Entering Zone " << zoneName << " Done" );
}

VOID RecordRoutineExit(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
    D1ECHO ("Exiting Routine : " << rname );

    // check first if map has entry for this ftn
    if ( symTable.IsSeenFunctionName(rname) )
    {
        IDNoType lastCallID = CallStack.Top();
        // check if the top ftn is the current one
        if ( lastCallID == FuncName2ID[rname] )
        {
            D1ECHO("Routine ID: " << FuncName2ID[rname]
                   << " Call stack top id " << lastCallID );

            D1ECHO("Leaving Routine : " << rname
                   << " Popping call stack top is "
                   << symTable.GetSymName( lastCallID ) );

            CallStack.Pop();
            CallSiteStack.Pop();
            #if (DEBUG>0)
            CallStack.Print();
            CallSiteStack.Print();
            #endif

            // In engine 3, to save time, the curr call is selected only at func entry/exit,
            // so that it does not need to be determined on each access
            if (KnobEngine.Value() == 3)
            {
                D1ECHO("Setting Current Call for : " << rname );
                SetCurrCallOnExit(lastCallID);
            }
        }
    }

    D1ECHO ("Exiting Routine : " << rname << " Done");
}

VOID RecordZoneExit(INT32 zoneNo)
{
    IDNoType lastCallID = CallStack.Top();
    string zoneName = symTable.GetSymName(lastCallID);
    D1ECHO ("Exiting Zone : " << zoneName );

    // check first if map has entry for this zone
    if( symTable.IsSeenFunctionName(zoneName) )
    {
        // check if the top ftn is the current one
        if ( lastCallID == FuncName2ID[zoneName] )
        {
            D1ECHO("Zone ID: " << FuncName2ID[zoneName]
                   << " Call stack top id " << lastCallID );

            D1ECHO("Leaving Zone : " << zoneName
                   << " Popping call stack top is "
                   << symTable.GetSymName( lastCallID ) );

            CallStack.Pop();
            CallSiteStack.Pop();
            #if (DEBUG>0)
            CallStack.Print();
            CallSiteStack.Print();
            #endif

            // In engine 3, to save time, the curr call is selected only at func entry/exit,
            // so that it does not need to be determined on each access
            if (KnobEngine.Value() == 3)
            {
                D1ECHO("Setting Current Call for : " << zoneName );
                SetCurrCallOnExit(lastCallID);
            }
        }
    }

    D1ECHO ("Exiting Zone : " << zoneName << " Done");
}

VOID SetCallSite(u32 locIndex)
{
    D2ECHO("setting last function call locIndex to " << locIndex << " "
                        << Locations.GetLocation(locIndex).toString());
    lastCallLocIndex=locIndex;
}

VOID MallocBefore(u32 size)
{
    D1ECHO("setting malloc size " << size );
    currSize = size;
}

VOID AllocaBefore(u32 size)
{
    D1ECHO("setting alloca size " << size );
    currSize = size;
}

VOID CallocBefore(u32 n, u32 size)
{
    D2ECHO("setting calloc size " << size );
    currSize = n*size;
}

// This is used for malloc, calloc and alloca
VOID MallocCallocAllocaAfter(uptr addr)
{
    D2ECHO("setting malloc/calloc/alloca start address " << ADDR(addr) << " and size " << currSize);
    currStartAddress = addr;

    symTable.InsertMallocCalloc(currStartAddress, lastCallLocIndex, currSize);
}

// TODO can reallocation decrease size?
VOID ReallocBefore(uptr addr, u32 size)
{
    D2ECHO("reallocation at address : " << ADDR(addr) );
    currStartAddress = addr;

    D2ECHO("setting realloc size " << size );
    currSize = size;
}

VOID ReallocAfter(uptr addr)
{
    D2ECHO("Setting Realloc address " << ADDR(addr) );
    uptr prevAddr = currStartAddress;
    if(prevAddr == 0) //realloc behaves like malloc, supplied null address as argument
    {
        currStartAddress = addr;
        symTable.InsertMallocCalloc(currStartAddress, lastCallLocIndex, currSize);
    }
    else
    {
        currID = GetObjectID(prevAddr);

        // check if relocation has moved the object to a different address
        if( addr != prevAddr )
        {
            D2ECHO("reallocation moved the object");
            D2ECHO("setting realloc start address " << ADDR(addr) );
            currStartAddress = addr;
        }

        symTable.UpdateRealloc(currID, currStartAddress, lastCallLocIndex, currSize);
    }
}

VOID FreeBefore(ADDRINT addr)
{
    D2ECHO("removing object with start address " << ADDR(addr) );
    symTable.Remove(addr);
}

VOID StrdupBefore(uptr addr)
{
    D2ECHO("setting strdup start address " << ADDR(addr) );
    currStartAddress = addr;
}

VOID StrdupAfter(uptr dstAddr)
{
    D2ECHO("setting strdup destination address " << ADDR(dstAddr) );
    uptr srcAddr = currStartAddress;
    currStartAddress = dstAddr;

    u32 currSize = symTable.GetSymSize(srcAddr);
    D2ECHO("strdup size: " << currSize);

    symTable.InsertMallocCalloc(currStartAddress, lastCallLocIndex, currSize);
}

// IMG instrumentation routine - called once per image upon image load
VOID InstrumentImages(IMG img, VOID * v)
{
    string imgname = IMG_Name(img);
    bool isLibC = imgname.find("/libc") != string::npos;

    // we should instrument malloc/free only when tracking objects !
    if ( TrackObjects && isLibC )
    {
        // instrument libc for malloc, free etc
        ECHO("Instrumenting "<<imgname<<" for (re)(c)(m)alloc/free routines etc ");

        RTN mallocRtn = RTN_FindByName(img, MALLOC.c_str() );
        if (RTN_Valid(mallocRtn))
        {
            RTN_Open(mallocRtn);
            D1ECHO("Instrumenting malloc");
            // Instrument malloc() to print the input argument value and the return value.
            RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)MallocBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocCallocAllocaAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(mallocRtn);
        }

        RTN allocaRtn = RTN_FindByName(img, ALLOCA.c_str() );
        if (RTN_Valid(allocaRtn))
        {
            RTN_Open(allocaRtn);
            D1ECHO("Instrumenting alloca");
            // Instrument malloc() to print the input argument value and the return value.
            RTN_InsertCall(allocaRtn, IPOINT_BEFORE, (AFUNPTR)AllocaBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_InsertCall(allocaRtn, IPOINT_AFTER, (AFUNPTR)MallocCallocAllocaAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(allocaRtn);
        }

        RTN callocRtn = RTN_FindByName(img, CALLOC.c_str() );
        if (RTN_Valid(callocRtn))
        {
            RTN_Open(callocRtn);
            D1ECHO("Instrumenting calloc");
            // Instrument calloc() to print the input argument value and the return value.
            RTN_InsertCall(callocRtn, IPOINT_BEFORE, (AFUNPTR)CallocBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                           IARG_END);
            // for now  using same callback ftns as for malloc
            RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)MallocCallocAllocaAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(callocRtn);
        }

        RTN reallocRtn = RTN_FindByName(img, REALLOC.c_str() );
        if (RTN_Valid(reallocRtn))
        {
            RTN_Open(reallocRtn);
            D1ECHO("Instrumenting realloc");
            // Instrument calloc() to print the input argument value and the return value.
            // for now  using same callback ftns as for malloc
            RTN_InsertCall(reallocRtn, IPOINT_BEFORE, (AFUNPTR)ReallocBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                           IARG_END);
            RTN_InsertCall(reallocRtn, IPOINT_AFTER, (AFUNPTR)ReallocAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(reallocRtn);
        }

        // Find the free() function.
        RTN freeRtn = RTN_FindByName(img, FREE.c_str() );
        if (RTN_Valid(freeRtn))
        {
            RTN_Open(freeRtn);
            D1ECHO("Instrumenting free");
            // Instrument free() to print the input argument value.
            RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)FreeBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_Close(freeRtn);
        }

        //  Find the strdup() function.
        RTN strdupRtn = RTN_FindByName(img, STRDUP.c_str() );
        if (RTN_Valid(strdupRtn))
        {
            RTN_Open(strdupRtn);
            D1ECHO("Instrumenting strdup");
            // Instrument strdup() to print the input argument value and the return value.
            RTN_InsertCall(strdupRtn, IPOINT_BEFORE, (AFUNPTR)StrdupBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_InsertCall(strdupRtn, IPOINT_AFTER, (AFUNPTR)StrdupAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(strdupRtn);
        }

    }

    // For simplicity, do rest of instrumentation only for main image.
    // This can be extended to any other image of course.
    if (    IMG_IsMainExecutable(img) == false &&
            KnobMainExecutableOnly.Value() == true
       )
    {
        ECHO("Skipping Image "<< imgname<< " for function calls instrumentation as it is not main executable");
        return;
    }
    else
    {
        ECHO("Instrumenting "<<imgname<<" for function calls as it is the Main executable ");
    }

    // Traverse the sections of the image.
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        // For each section, process all RTNs.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Many RTN APIs require that the RTN be opened first.
            RTN_Open(rtn);

            // Traverse all instructions
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                D2ECHO("disassembeled ins = " << INS_Disassemble(ins) );

                if( INS_IsDirectCall(ins) ) //INS_IsDirectBranchOrCall  OR  INS_IsCall(ins)
                {
                    ADDRINT target = INS_DirectBranchOrCallTargetAddress(ins);
                    string tname1 = Target2RtnName(target);
                    string tname = PIN_UndecorateSymbolName( tname1, UNDECORATION_NAME_ONLY);

                    u32 locIndex =-1;
                    string filename("");    // This will hold the source file name.
                    INT32 line = 0;     // This will hold the line number within the file
                    PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &filename);
                    Location loc(line, filename);

                    D1ECHO("  Routine Call found for " << tname << " at " << filename <<":"<< line);
                    bool inStdHeaders = ( filename.find("/usr/include") != string::npos );
                    if( (!inStdHeaders) && ( ValidFtnCallName(tname) ) )
                    {
                        bool found = Locations.GetLocIndexIfAvailable(loc, locIndex);
                        if( !found )
                            locIndex = Locations.Insert(loc);

                        D1ECHO("  Instrumenting call " << " at " << VAR(locIndex) << " " << filename <<":"<< line);

                        INS_InsertCall(
                            ins,
                            IPOINT_BEFORE,
                            (AFUNPTR)SetCallSite,
                            IARG_UINT32, locIndex,
                            IARG_END
                        );

                    }
                }

                if (INS_IsRet(ins))
                {
                    INS_InsertCall(
                        ins,
                        IPOINT_BEFORE,
                        (AFUNPTR)RecordRoutineExit,
                        IARG_INST_PTR,
                        IARG_END
                    );
                }
            }
            RTN_Close(rtn); // Don't forget to close the RTN once you're done.
        }
    }
}

VOID Markers( INT32 arg, INT32 arg1, INT32 arg2)
{
    int cmd = (arg & __PIN_CMD_MASK) >> __PIN_CMD_OFFSET;
    int val = arg & __PIN_ID_MASK;

    D2ECHO("Recieved MAGIC " << VAR(cmd) << VAR(val) << VAR(arg) << VAR(arg1) );

    switch(cmd)
    {
    case __PIN_MAGIC_CMD_NOARG:
        switch (val)
        {
            case __PIN_MAGIC_START:
                if(TrackStartStop)
                {
                    D1ECHO("__PIN_MAGIC_START");
                    SelectAnalysisEngine();
                }
            break;
            case __PIN_MAGIC_STOP:
                if(TrackStartStop)
                {
                    D1ECHO("__PIN_MAGIC_STOP");
                    SelectDummyAnalysisEngine();
                }
            break;
            default:
                ECHO("Unknown NOARG MAGIC " << val);
            break;
        }
    break;
    case __PIN_MAGIC_ZONE_ENTER:
        if(TrackZones)
        {
            D1ECHO("__PIN_MAGIC_ZONE_ENTER");
            RecordZoneEntry(val);
        }
    break;
    case __PIN_MAGIC_ZONE_EXIT:
        if(TrackZones)
        {
            D1ECHO("__PIN_MAGIC_ZONE_EXIT");
            RecordZoneExit(val);
        }
    break;
    default:
        ECHO("Unknown ARG MAGIC " << cmd);
    break;
    }
}

VOID InstrumentTraces(TRACE trace, VOID *v)
{
//     RTN rtn = TRACE_Rtn (trace);
//     if (! RTN_Valid (rtn))
//         return;
//     ECHO( "This trace belongs to routine:" << RTN_Name(rtn) );

    for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl=BBL_Next(bbl))
    {
        // Insert a call to InstrCounter for every bbl, passing the number of instructions.
        // IPOINT_ANYWHERE allows Pin to schedule the call anywhere in the bbl to obtain best performance.
        // Use a fast linkage for the call.
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(InstrCounter), IARG_FAST_ANALYSIS_CALL,
                       IARG_UINT32, BBL_NumIns(bbl),
                       IARG_END);

        for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins=INS_Next(ins))
        {
            D2ECHO("Dissassembled ins : " << INS_Disassemble(ins) );
            bool isStackRead = INS_IsStackRead(ins)  || INS_IsIpRelRead(ins) ;
            bool isStackWrite = INS_IsStackWrite(ins) || INS_IsIpRelWrite(ins);
            bool ignoreStack = !KnobRecordStack.Value();

            // Instrument Read accesses
            if( ignoreStack )
            {
                if ( isStackRead )
                {
                    D2ECHO("Ignoring as stack");
                }
                else if (INS_IsMemoryRead(ins) )
                {
                    D2ECHO("Recording heap");
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                        IARG_MEMORYREAD_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );

                    if (INS_HasMemoryRead2(ins) )
                    {
                        INS_InsertPredicatedCall
                        (
                            ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                            IARG_MEMORYREAD2_EA,
                            IARG_MEMORYREAD_SIZE,
                            //IARG_UINT32, INS_IsPrefetch(ins),
                            IARG_END
                        );
                    }
                }
            }
            else
            {
                D2ECHO("Recording heap (not ignoring)");
                if (INS_IsMemoryRead(ins) )
                {
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                        IARG_MEMORYREAD_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );
                }

                if (INS_HasMemoryRead2(ins) )
                {
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                        IARG_MEMORYREAD2_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );
                }
            }

            // Instrument Write accesses
            if( ignoreStack )
            {
                if ( isStackWrite)
                {
                }
                else if (INS_IsMemoryWrite(ins) )
                {
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)WriteRecorder,
                        IARG_MEMORYWRITE_EA,
                        IARG_MEMORYWRITE_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );
                }
            }
            else
            {
                if (INS_IsMemoryWrite(ins) )
                {
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)WriteRecorder,
                        IARG_MEMORYWRITE_EA,
                        IARG_MEMORYWRITE_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );
                }
            }

            if( TrackZones || TrackStartStop ) // If marker tracking is requested
            {
                if (INS_Disassemble(ins) == "xchg bx, bx") // track the magic instruction
                {
                    // RTN rtn = INS_Rtn(ins);
                    // string rtnName = RTN_Name(rtn);
                    D2ECHO("Instrumenting XCHG/Markers instruction in " << RTN_Name(INS_Rtn(ins)) << "()" );
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)Markers,
                                             IARG_REG_VALUE, REG_EAX,
                                             IARG_REG_VALUE, REG_ECX,
                                             IARG_REG_VALUE, REG_EDX,
                                             IARG_END);
                }
            }
        } // ins
    } // BBL
}

VOID PrintRtnNameOnly(CHAR* rname)
{
    ECHO ("Executed Routine : " << rname );
}

VOID InstrumentRoutines(RTN rtn, VOID *v)
{
    /*
     * The following function recording can be done at the instrumentation time as
     * below instead of doing at analysis time in RecordRoutineEntry(). This has
     * resulted in more functions in SeenFnames which may not be even involved in
     * communication. This, however, is not as such a problem as it will simply
     * clutter the output, and easily be cleaned in dot file etc.
     */

    IMG img = SEC_Img(RTN_Sec(rtn));

    if( IMG_IsMainExecutable(img) )
    {
        string mangRName = RTN_Name(rtn);
        string rname = PIN_UndecorateSymbolName( mangRName, UNDECORATION_NAME_ONLY);
        //string rname = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_COMPLETE);

        if (ValidFtnName(rname))
        {
            if( KnobSelectFunctions.Value() )
            {
                // In Select Function mode, functions are added a priori from
                // the list file to symbol table. So, if a function is not
                // found in symbol table, it means it is not in select ftn list
                // so it should be skiped
                if( !symTable.IsSeenFunctionName(rname) )
                {
                    D1ECHO ("Skipping Instrumentation of un-selected Routine : " << rname);
                    return;
                }
            }
            else
            {
                // First time seeing this valid function name, save it in the list
                symTable.InsertFunction(rname);
            }

            D1ECHO ("Instrumenting Routine : " << rname);
            RTN_Open(rtn);

            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RecordRoutineEntry,
                           IARG_ADDRINT, rname.c_str(),
                           IARG_END);

            RTN_Close(rtn);
        }
        else
        {
            D1ECHO ("Skipping Instrumentation of Invalid Routine : " << rname);
        }
    }

    /*
    // for testing various routines in images other than main executable
    else
    {
        string imagename = IMG_Name(img).c_str();
    //         ECHO("Image Name " << imagename);
    //         bool isLibC = imagename.find("/libc") != string::npos;
    //         if(isLibC)
        {
            string rname = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_NAME_ONLY);
    //             ECHO ("Seen Routine : " << rname);
            RTN_Open(rtn);

            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)PrintRtnNameOnly,
                           IARG_ADDRINT, rname.c_str(),
                           IARG_END);

            RTN_Close(rtn);
        }
    }
    */
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID TheEnd(INT32 code, VOID *v)
{
    symTable.Print();
#if (DEBUG>0)
    // Print Symbol Table to output file
    symTable.Print();
#endif

    switch( KnobEngine.Value() )
    {
    case 1:
        PrintAccesses();
        break;
    case 2:
        if(TrackObjects)
        {
            OpenOutFile(KnobDotFile.Value(), dotout);
            ComMatrix.PrintDot(dotout);
            dotout.close();
        }
        else
        {
            OpenOutFile(KnobDotFile.Value(), dotout);
            OpenOutFile(KnobMatrixFile.Value(), mout);
            ComMatrix.PrintMatrix(mout);
            ComMatrix.PrintDot(dotout);
            mout.close();
            dotout.close();
        }
        break;
    case 3:
        PrintAllCalls(pcout);
        pcout.close();
        break;
    default:
        ECHO("Specify a valid Engine number to be used");
        Die();
        break;
    }

    //PrintInstrCount();
    //PrintInstrPercents();
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments,
 *                              including pin -t <toolname> -- ...
 */
void SetupPin(int argc, char *argv[])
{
    PIN_InitSymbols();
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if( PIN_Init(argc,argv) )
    {
        Usage();
        Die();
    }

    // TODO may be this can be pushed in constructor of symTable
    // furthermore, unknownObj can also be pushed!!!
    // Insert Unknown Ftn as first symbol
    symTable.InsertFunction(UnknownFtn);

    // Push the first ftn as UNKNOWN
    // The name can be adjusted from globals.h
    CallStack.Push(FuncName2ID[UnknownFtn]);

    if(KnobSelectFunctions.Value())
    {
        ECHO("Selected Functions Feature is not complete...");
        Die();
        //D1ECHO("Initialize functions from Selected Functions file ...");
        //symTable.InitFromFtnFile();
    }

    if(KnobSelectObjects.Value() )
    {
        ECHO("Selected Objects Feature is not complete...");
        Die();
        //D1ECHO("Initialize objects from Selected Objects file ...");
        //symTable.InitFromObjFile();
    }

    TrackObjects = KnobTrackObjects.Value();
    RecordAllAllocations=KnobRecordAllAllocations.Value();
    FlushCalls=KnobFlushCalls.Value();
    FlushCallsLimit=KnobFlushCallsLimit.Value();
    ShowUnknown = KnobShowUnknown.Value();
    TrackStartStop = KnobTrackStartStop.Value();
    TrackZones = KnobTrackZones.Value();

#if (DEBUG>0)
    ECHO("Printing Initial Symbol Table ...");
    symTable.Print();
#endif

    D1ECHO("Selecting Analysis Engine ...");
    if(TrackStartStop)
    {
        // start dummy so that actual profiling starts when start is seen
        SelectDummyAnalysisEngine();
    }
    else
    {
        // else start profiling right away
        SelectAnalysisEngine();
    }

    if ( KnobEngine.Value() == 3 )
    {
        OpenOutFile(KnobPerCallFile.Value(), pcout);
    }

    // Register function for Image-level instrumentation
    IMG_AddInstrumentFunction(InstrumentImages, 0);
    // Register function for Routine-level instrumentation
    RTN_AddInstrumentFunction(InstrumentRoutines, 0);
    // Register function for Trace-level instrumentation
    TRACE_AddInstrumentFunction(InstrumentTraces, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(TheEnd, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
