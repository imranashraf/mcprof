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
 * Copyright (c) 2014-2016 TU Delft, The Netherlands.
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

#include "pin.H"
#include "markers.h"
#include "globals.h"
#include "symbols.h"
#include "pintrace.h"
#include "commatrix.h"
#include "sparsematrix.h"
#include "shadow.h"
#include "callstack.h"
#include "engine1.h"
#include "engine2.h"
#include "engine3.h"
#include "counters.h"
#include "callgraph.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <set>
#include <map>
#include <deque>
#include <algorithm>
#include <cstring>
#include <cstddef>

// un-comment the following to generate read/write traces (Also in engine2.cpp)
#define GENRATE_TRACES

/* ================================================================== */
// Global variables
/* ================================================================== */
extern map <string,IDNoType> FuncName2ID;
extern Symbols symTable;
extern Matrix2D ComMatrix;
extern SparseMatrix DependMatrix;
extern map <u32,IDNoType> CallSites2ID;

std::ofstream pcout;
std::ofstream tgout;
std::ofstream traceout;

// these maps are used to record each execution of loop as dependent/independent
set<string>dependentLoopExecutions;
set<string>independentLoopExecutions;

// this map is used to record recursive functions
set<string>recursiveFunctions;

CallStackType CallStack;
CallSiteStackType CallSiteStack;

void (*WriteRecorder)(uptr, u32);
void (*ReadRecorder)(uptr, u32);
void (*InstrCounter)(ADDRINT);
VOID (*RoutineEntryRecorder)(ADDRINT);
VOID (*RoutineExitRecorder)(VOID*);

u32 Engine=0;
bool DoTrace=true;
bool TrackObjects;
bool RecordAllAllocations;
bool FlushCalls;
u32  FlushCallsLimit;
bool ShowUnknown;
bool TrackZones;
bool TrackStartStop;
bool TrackLoopDepend;
bool TrackTasks;
u32  Threshold;

static UINT64 rInstrCount = 0; // Routine instruction counter
UINT64 gInstrCount = 0; // Global running instruction counter

CallGraph callgraph;

u32 SelectedLoopNo;
extern map<IDNoType,u64> instrCounts;
extern map<IDNoType,u64> callCounts;

u32 LoopIterationCount=1;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */

KNOB<string> KnobPerCallFile(KNOB_MODE_WRITEONCE,  "pintool",
                         "PerCallFile", "percallaccesses.dat",
                         "specify file name for per call output file");

KNOB<BOOL> KnobRecordStack(KNOB_MODE_WRITEONCE, "pintool",
                           "RecordStack","0", "Include Stack Accesses");

KNOB<BOOL> KnobTrackObjects(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackObjects", "0", "Track the objects");

KNOB<UINT32> KnobEngine(KNOB_MODE_WRITEONCE,  "pintool",
                        "Engine", "1",
                        "specify engine number (1,2,3) to be used");

KNOB<BOOL> KnobSelectFunctions(KNOB_MODE_WRITEONCE, "pintool",
                               "SelectFunctions", "0",
                               "Instrument only the selected functions.\
                                User provides functions in <SelectFunctions.txt> file");

KNOB<BOOL> KnobSelectObjects(KNOB_MODE_WRITEONCE, "pintool",
                             "SelectObjects", "0",
                             "Instrument only the selected objects.\
                              User provides objects in <SelectObjects.txt> file");

KNOB<BOOL> KnobMainExecutableOnly(KNOB_MODE_WRITEONCE, "pintool",
                                  "MainExecOnly","1",
                                  "Trace functions that are contained only in the\
                                  executable image");

KNOB<BOOL> KnobRecordAllAllocations(KNOB_MODE_WRITEONCE, "pintool",
                                  "RecordAllAllocations","1",
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
                            "TrackStartStop", "0", "Track start/stop markers in\
                            the code to start/stop profiling\
                            instead of starting from main()");

KNOB<BOOL> KnobTrackZones(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackZones", "0", "Track zone markers to profile per zones");

KNOB<BOOL> KnobTrackLoopDepend(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackLoopDepend", "0", "Track markers for loop dependence");

KNOB<UINT32> KnobSelectedLoopNo(KNOB_MODE_WRITEONCE,  "pintool",
                        "SelectedLoopNo", "0",
                        "Specify loop no to test of dependence.");

KNOB<BOOL> KnobReadStaticObjects(KNOB_MODE_WRITEONCE, "pintool",
                            "StaticSymbols", "0", "Read static symbols from the binary and show them in the graph");

KNOB<BOOL> KnobTrackTasks(KNOB_MODE_WRITEONCE, "pintool",
                            "TrackTasks", "0", "Each function call and loop execution for different source-code\
                            location will be considered as separate tasks.");

KNOB<UINT32> KnobThreshold(KNOB_MODE_WRITEONCE,  "pintool",
                        "Threshold", "0",
                        "Specify Threshold, communication edge below this Threshold \
                        will not appear in the communication graph/matrix.");


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
    if(DoTrace)
    {
        IDNoType fid = CallStack.Top();
        instrCounts[fid] += c;
        rInstrCount+=c;
    }
}

VOID doInstrCountGlobal()
{
    if(DoTrace)
        ++gInstrCount;
}

VOID Instruction(INS ins, VOID *v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)doInstrCountGlobal, IARG_END);
}

void dummyRecorder(uptr a, u32 b){}
VOID dummyInstrCounter(ADDRINT c){}

void SelectAnalysisEngine()
{
    InstrCounter = doInstrCount;
    switch( Engine )
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
    ECHO("Selected Engine number " << Engine);
    D2ECHO( ADDR((void*)RecordReadEngine1) << " and " << ADDR((void*)RecordWriteEngine1) );
    D2ECHO( ADDR((void*)RecordReadEngine2) << " and " << ADDR((void*)RecordWriteEngine2) );
    D2ECHO( ADDR((void*)RecordReadEngine3) << " and " << ADDR((void*)RecordWriteEngine3) );
}

void SelectDummyAnalysisEngine()
{
    ReadRecorder = dummyRecorder;
    WriteRecorder = dummyRecorder;
    InstrCounter = dummyInstrCounter;
    ECHO("Selected Engine dummy");
    D2ECHO( ADDR((void*)dummyRecorder) );
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
            (name.find("::operator") != string::npos) ||
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
            !name.compare(".gnu.version") ||
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
            !name.compare(".gnu.version") ||
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

VOID RoutineEntryRecorder1(ADDRINT irname)
{
    const char* rname = reinterpret_cast<const char *>(irname);
    D1ECHO ("RoutineEntryRecorder1 : " << rname );
    string calleeName(rname);
    IDNoType calleeID=0;

    if( !symTable.IsSeenFunctionName(calleeName) )
    {
        calleeID = GetNewID();
        symTable.InsertFunction(calleeName, calleeID, lastCallLocIndex);
    }

    calleeID = FuncName2ID[calleeName];

    // following is to store recursive functions
    IDNoType callerID = CallStack.Top();
    string callerName = symTable.GetSymName(callerID);
    if(callerName != calleeName) // Non recursive function
    {
        CallStack.Push(calleeID);
        CallSiteStack.Push(lastCallLocIndex);
    }
    else
    {
        recursiveFunctions.insert(calleeName);
    }

    // call count should be updated even for recursive functions
    callCounts[calleeID] += 1;
    callgraph.UpdateCall(calleeID, rInstrCount);
    rInstrCount=0;

    D1ECHO ("Entered Routine1 : " << calleeName << " with ID " << calleeID);
    #if (DEBUG>0)
    CallStack.Print();
    //CallSiteStack.Print();
    #endif

    // In engine 3, to save time, the curr call is selected only at
    // func entry/exit, so that it does not need to be determined on each access
    if ( Engine == 3)
    {
        D1ECHO ("Setting Current Call for : " << rname );
        SetCurrCallOnEntry();
    }

    D1ECHO ("RoutineEntryRecorder1 : " << calleeName << " Done" );
}

// This recorder is used when tasks are tracked. The main differences are :
//    new id is generated for each new call site
//    and the way these are named (required for mcpar)
VOID RoutineEntryRecorder2(ADDRINT irname)
{
    const char* rname = reinterpret_cast<const char *>(irname);
    string calleeName(rname);
    D1ECHO ("RoutineEntryRecorder2 : " << calleeName );

    IDNoType callerID = CallStack.Top();
    string callerName = symTable.GetSymName(callerID);
    IDNoType tempID=0;
    string callerNameSimple(callerName);
    RemoveNoFromNameEnd(callerNameSimple, tempID); // remove previous id
    RemoveNoFromNameEnd(callerNameSimple, tempID); // remove previous lastCallLocIndex

    IDNoType calleeID=0;
    D2ECHO( callerNameSimple << " ->  " << calleeName);
    if( callerNameSimple != calleeName ) // Non recursive case
    {
        // for each callsite a unique id is generated
        // this will result in the generation of unique name
        if( !GetAvailableORNewID(calleeID, lastCallLocIndex) ) // returns false if id is new
        {
            AddNoToNameEnd(calleeName, lastCallLocIndex);
            AddNoToNameEnd(calleeName, calleeID);
            symTable.InsertFunction(calleeName, calleeID, lastCallLocIndex);

            // for taskgraph output
            tgout << callerName << " FUNC " << calleeName  << "\n";
        }

        D1ECHO ("Using ID " << calleeID);
        CallStack.Push(calleeID);
        CallSiteStack.Push(lastCallLocIndex);
        D1ECHO ("Entered Routine2 : " << calleeName << " with ID " << calleeID);

        callCounts[calleeID] += 1;
        callgraph.UpdateCall(calleeID, rInstrCount);
        rInstrCount=0;
    }
    else // recursive case
    {
        // call count should be updated even for recursive functions
        // in this case same callerID should be used
        callCounts[callerID] += 1;
        callgraph.UpdateCall(callerID, rInstrCount);
        rInstrCount=0;
    }

    #if (DEBUG>0)
    CallStack.Print();
    //CallSiteStack.Print();
    #endif

    // TODO test with updated logic later
    // In engine 3, to save time, the curr call is selected only at
    // func entry/exit, so that it does not need to be determined on each access
    if ( Engine == 3)
    {
        D1ECHO ("Setting Current Call for : " << calleeName );
        SetCurrCallOnEntry();
    }

    D1ECHO ("RoutineEntryRecorder2 : " << calleeName << " Done" );
}

VOID RoutineExitRecorder1(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
    D1ECHO ("RoutineExitRecorder1 : " << rname );

    // check first if map has entry for this ftn
    if ( symTable.IsSeenFunctionName(rname) )
    {
        IDNoType lastCallID = CallStack.Top();
        // check if the top ftn is the current one
        if ( lastCallID == FuncName2ID[rname] )
        {
            D1ECHO("Leaving Routine : " << rname << " id : " << FuncName2ID[rname]);
            D1ECHO("Will pop : " << symTable.GetSymName( lastCallID ) << " id : " << lastCallID );

            CallStack.Pop();
            CallSiteStack.Pop();

            D1ECHO ("Exited Routine1 : " << rname << " rInstrCount = " << rInstrCount);

            #if (DEBUG>0)
            CallStack.Print();
            //CallSiteStack.Print();
            #endif

            // In engine 3, to save time, the curr call is selected only at func entry/exit,
            // so that it does not need to be determined on each access
            if ( Engine == 3)
            {
                D1ECHO("Setting Current Call for : " << rname );
                SetCurrCallOnExit(lastCallID);
            }
        }

        // call count should be updated even for recursive functions
        callgraph.UpdateReturn( lastCallID, rInstrCount );
        rInstrCount = 0;
    }

    // un-comment the following to enable printing running count of instructions executed so far
    //ECHO("Instructions executed so far : " << rInstrCount );
    D1ECHO ("RoutineExitRecorder1 : " << rname << " Done");
}

// Used when tracking tasks
VOID RoutineExitRecorder2(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
    D1ECHO ("RoutineExitRecorder2 : " << rname );

    if( ValidFtnName(rname) )
    {
        string calleeName(rname); // name of routine which currently wants to exit

        IDNoType tempID=0;
        IDNoType topRtnID = CallStack.Top();
        string topRtnName = symTable.GetSymName( topRtnID );
        RemoveNoFromNameEnd(topRtnName, tempID); // remove previous id
        RemoveNoFromNameEnd(topRtnName, tempID); // remove previous lastCallLocIndex

        if ( calleeName == topRtnName ) // check if current == actual
        {
            CallStack.Pop();
            CallSiteStack.Pop();

            D1ECHO ("Exited2 Routine : " << rname << " rInstrCount = " << rInstrCount);

            // TODO test it later with updated logic
            // In engine 3, to save time, the curr call is selected only at func entry/exit,
            // so that it does not need to be determined on each access
            if ( Engine == 3 )
            {
                D1ECHO("Setting Current Call for : " << rname );
                SetCurrCallOnExit(topRtnID);
            }
        }

        // call count should be updated even for recursive functions
        callgraph.UpdateReturn( topRtnID, rInstrCount );
        rInstrCount = 0;
    }

    #if (DEBUG>0)
    CallStack.Print();
    //CallSiteStack.Print();
    #endif

    // un-comment the following to enable printing running count of instructions executed so far
    //ECHO("Instructions executed so far : " << rInstrCount );

    D1ECHO ("RoutineExitRecorder2 : " << rname << " Done");
}

VOID RecordZoneEntry(INT32 zoneNo)
{
    IDNoType callerID = CallStack.Top();
    string callerName = symTable.GetSymName(callerID);
    D1ECHO("RecordZoneEntry : " << callerName);

    // for each callsite a unique id is generated
    // this will result in the generation of unique name
    IDNoType calleeID=0;
    string calleeName(callerName);
    bool isNewID = !GetAvailableORNewID(calleeID, lastCallLocIndex); // returns false if id is new
    if( isNewID )
    {
        D2ECHO(" Using new id " << calleeID << " for : " << calleeName);
        if(TrackTasks || TrackLoopDepend)
        {
            IDNoType tempCalleeID=0;
            IDNoType tempLastCallLocIndex=0;
            RemoveNoFromNameEnd(calleeName, tempCalleeID); // remove previous id if there
            RemoveNoFromNameEnd(calleeName, tempLastCallLocIndex); // remove previous lastCallLocIndex if there
            AddNoToNameEnd(calleeName, lastCallLocIndex); // attach new lastCallLocIndex
            AddNoToNameEnd(calleeName, calleeID); // attach new id

            // for taskgraph output
            tgout << callerName << " LOOP " << calleeName << "\n";
        }
        else
        {
            calleeName += "_Zone" + to_string(zoneNo);
        }

        symTable.InsertFunction(calleeName, calleeID, lastCallLocIndex);
    }

    CallStack.Push(calleeID);
    CallSiteStack.Push(lastCallLocIndex);
    D1ECHO("Entered zone : " << calleeName << " with ID " << calleeID);

    callCounts[calleeID] += 1;
    callgraph.UpdateCall(calleeID, rInstrCount);
    rInstrCount=0;

    #if (DEBUG>0)
    CallStack.Print();
    //CallSiteStack.Print();
    #endif

    // TODO test with updated logic later
    // In engine 3, to save time, the curr call is selected only at
    // func entry/exit, so that it does not need to be determined on each access
    if ( Engine == 3 )
    {
        D1ECHO ("Setting Current Call for : " << calleeName );
        SetCurrCallOnEntry();
    }

    D1ECHO ("RecordZoneEntry : " << calleeName << " Done");
}

VOID RecordZoneExit(INT32 zoneNo)
{
    IDNoType lastCallID = CallStack.Top();
    string zoneName = symTable.GetSymName(lastCallID);
    D1ECHO ("RecordZoneExit : " << zoneName );

    // check first if map has entry for this zone
    if( symTable.IsSeenFunctionName(zoneName) )
    {
        // check if the top ftn is the current one
        if ( lastCallID == FuncName2ID[zoneName] )
        {
            D1ECHO("Leaving Zone : " << zoneName << " id : " << FuncName2ID[zoneName] );
            D1ECHO("Will pop : " << symTable.GetSymName( lastCallID ) << " id : " << lastCallID );

            CallStack.Pop();
            CallSiteStack.Pop();
            D1ECHO("Exited zone : " << zoneName << " with ID " << lastCallID << " rInstrCount = " << rInstrCount);
            callgraph.UpdateReturn( lastCallID, rInstrCount );
            rInstrCount = 0;

            #if (DEBUG>0)
            CallStack.Print();
            //CallSiteStack.Print();
            #endif

            // In engine 3, to save time, the curr call is selected only at func entry/exit,
            // so that it does not need to be determined on each access
            if ( Engine == 3 )
            {
                D1ECHO("Setting Current Call for : " << zoneName );
                SetCurrCallOnExit(lastCallID);
            }
        }
    }

    D1ECHO ("RecordZoneExit : " << zoneName << " Done");
}

VOID SetCallSite(u32 locIndex)
{
    D2ECHO("setting last callsite to : " << Locations.GetLocation(locIndex).toString() << " locIndex : " << locIndex);
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

        symTable.UpdateRealloc(currID, prevAddr, addr, lastCallLocIndex, currSize);
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

    // We should instrument malloc/free only when tracking objects !
    // We should also track them when tracking tasks to report
    // allocation dependencies.
    if ( (TrackObjects || TrackTasks) && isLibC )
//     if ( (TrackObjects || (TrackTasks && !TrackLoopDepend) ) && isLibC )
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
                    string fileName("");    // This will hold the source file name.
                    INT32 line = 0;     // This will hold the line number within the file
                    PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &fileName);
                    // Remove the complete path of the filename
                    if(fileName == "")  fileName = "NA";
                    else RemoveCurrDirFromName(fileName);
                    // create a temp Location loc, may be inserted in list of locations later
                    Location loc(line, fileName);
                    D1ECHO("  Routine Call found for " << tname << " at " << fileName <<":"<< line);
                    bool inStdHeaders = ( fileName.find("/usr/include") != string::npos );
                    if( (!inStdHeaders) && ( ValidFtnCallName(tname) ) )
                    {
                        bool found = Locations.GetLocIndexIfAvailable(loc, locIndex);
                        if( !found )
                        {
                            D2ECHO("Location " << loc.toString() << " not found, Inserting");
                            locIndex = Locations.Insert(loc);
                        }

                        D1ECHO("  Instrumenting call " << " at " << VAR(locIndex) << " " << fileName <<":"<< line);

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
                        (AFUNPTR)RoutineExitRecorder,
                        IARG_INST_PTR,
                        IARG_END
                    );
                }
            }
            RTN_Close(rtn); // Don't forget to close the RTN once you're done.
        }
    }
}

VOID Markers(INT32 locidx, INT32 arg, INT32 arg1, INT32 arg2)
{
    u32 cmd = (arg & __PIN_CMD_MASK) >> __PIN_CMD_OFFSET;
    u32 val = arg & __PIN_ID_MASK;
    D2ECHO("Recieved MAGIC " << VAR(cmd) << VAR(val) << VAR(arg) << VAR(arg1) );

    switch(cmd)
    {
    case __PIN_MAGIC_CMD_NOARG:
        switch (val)
        {
            case __PIN_MAGIC_START:
                if(TrackStartStop)
                {
                    D2ECHO("__PIN_MAGIC_START");
                    DoTrace=true;
                }
            break;
            case __PIN_MAGIC_STOP:
                if(TrackStartStop)
                {
                    D2ECHO("__PIN_MAGIC_STOP");
                    DoTrace=false;
                }
            break;
            default:
                ECHO("Unknown NOARG MAGIC " << val);
            break;
        }
    break;
    case __PIN_MAGIC_ZONE_ENTER:
        D1ECHO("__PIN_MAGIC_ZONE_ENTER");
        if( TrackZones || TrackLoopDepend )
        {
            D2ECHO("setting call site locindex to " << locidx);
            SetCallSite(locidx);
            RecordZoneEntry(val);
        }
        if( TrackLoopDepend && val==SelectedLoopNo )
        {
            LoopIterationCount=1; // '0' refers to R/W before the start of loop.
                                  //  These get associated with unknown.
            DependMatrix.Clear(); // TODO check if later if its needed
        }
    break;
    case __PIN_MAGIC_ZONE_EXIT:
    {
        D1ECHO("__PIN_MAGIC_ZONE_EXIT");
        IDNoType loopID;
        string loopName;
        if( TrackZones || TrackLoopDepend )
        {
            // before exiting, we need to note the name of the exiting zone/loop to print
            // its dependence information later
            loopID = CallStack.Top();
            loopName = symTable.GetSymName(loopID);
            RecordZoneExit(val);
        }
        if( TrackLoopDepend && val==SelectedLoopNo )
        {
            bool result = DependMatrix.CheckLoopIndependence(LoopIterationCount);
            if(result)
            { // independent
                independentLoopExecutions.insert(loopName);
            }
            else
            { // dependent
                dependentLoopExecutions.insert(loopName);
            }
            //DependMatrix.PrintMatrix(LoopIterationCount);
            //DependMatrix.PrintMatrixAsSparse(LoopIterationCount);
        }
    }
    break;
    case __PIN_MAGIC_LOOPBODY_ENTER:
        if( TrackLoopDepend && val==SelectedLoopNo )
        {
            DoTrace=true;
            D1ECHO("__PIN_MAGIC_LOOPBODY_ENTER  for Loop No : " << val << " Iterations : " << LoopIterationCount);
        }
    break;
    case __PIN_MAGIC_LOOPBODY_EXIT:
        if( TrackLoopDepend && val==SelectedLoopNo )
        {
            D1ECHO("__PIN_MAGIC_LOOPBODY_EXIT for Loop No : " << val << " Iterations : " << LoopIterationCount-1);
            DoTrace=false;
            ++LoopIterationCount;
        }
    break;
    default:
        ECHO("Unknown MAGIC ARG" << cmd);
    break;
    }
}

//#define USEDEBUGENGINE2
// Above define can be used to perform detailed debug of engine 2.
// Prints detailed output for dependencies.
VOID InstrumentTraces(TRACE trace, VOID *v)
{
    RTN rtn = TRACE_Rtn (trace);
    if (! RTN_Valid (rtn))
        return;
    D2ECHO( "This trace belongs to routine:" << RTN_Name(rtn) );

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
            if( TrackZones || TrackStartStop || TrackLoopDepend ) // If marker tracking is requested
            {
                if (INS_Disassemble(ins) == "xchg bx, bx") // track the magic instruction
                {
                    // RTN rtn = INS_Rtn(ins);
                    // string rtnName = RTN_Name(rtn);
                    string fileName("");    // This will hold the source file name.
                    INT32 line = 0;     // This will hold the line number within the file
                    PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &fileName);
                    // Remove the complete path of the filename
                    if(fileName == "")  fileName = "NA";
                    else RemoveCurrDirFromName(fileName);
                    D2ECHO("Marker at " << fileName << ":" << line);
                    // create a temp Location loc, may be inserted in list of locations later
                    Location loc(line+1, fileName); // 1 is added as offset as loop start at next line of marker
                    u32 locIndex =-1;
                    bool found = Locations.GetLocIndexIfAvailable(loc, locIndex);
                    if( !found )
                    {
                        D2ECHO("Marker Location " << loc.toString() << " not found, Inserting");
                        locIndex = Locations.Insert(loc);
                    }

                    D2ECHO("Instrumenting XCHG/Markers instruction in " << RTN_Name(INS_Rtn(ins)) << "()" );
                    INS_InsertPredicatedCall
                        ( ins, IPOINT_BEFORE,
                          (AFUNPTR)Markers,
                          IARG_UINT32, locIndex,
                          IARG_REG_VALUE, REG_EAX,
                          IARG_REG_VALUE, REG_ECX,
                          IARG_REG_VALUE, REG_EDX,
                          IARG_END
                        );
                }
            }

            // skip instrumenting call/return/prefetch instructions for memory accesses
            if( INS_IsCall(ins) || INS_IsRet(ins) || INS_IsPrefetch(ins) ) continue;

#ifdef USEDEBUGENGINE2
            char* cstr = new char[ 100 ];
            string insTemp = INS_Disassemble(ins);
            strcpy(cstr, insTemp.c_str() );
#endif

            D2ECHO("Dissassembled ins : " << INS_Disassemble(ins) );
            bool isStackRead = INS_IsStackRead(ins)  || INS_IsIpRelRead(ins) ;
            bool isStackWrite = INS_IsStackWrite(ins) || INS_IsIpRelWrite(ins);
            bool ignoreStack = !( KnobRecordStack.Value() );
            bool traceRead = !( isStackRead & ignoreStack );
            bool traceWrite = !( isStackWrite & ignoreStack );

            D2ECHO( VAR(traceRead) << VAR(traceWrite) );
            D2ECHO( VAR(INS_IsMemoryRead(ins)) << VAR(INS_IsMemoryWrite(ins)) );

            // Instrument Read accesses
            if( traceRead )
            {
                if (INS_IsMemoryRead(ins) )
                {
                    D2ECHO("Instrumenting Read in ins : " << INS_Disassemble(ins) );
#ifdef USEDEBUGENGINE2
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordReadEngine4Debug,
                        IARG_MEMORYREAD_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_ADDRINT, (ADDRINT)(cstr),
                        IARG_INST_PTR,
                        IARG_END
                    );
#else
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                        IARG_MEMORYREAD_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );
#endif
                }
                if (INS_HasMemoryRead2(ins) )
                {
                    D2ECHO("Instrumenting Read2 in ins : " << INS_Disassemble(ins) );
#ifdef USEDEBUGENGINE2
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordReadEngine4Debug,
                        IARG_MEMORYREAD2_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_ADDRINT, (ADDRINT)(cstr),
                        IARG_INST_PTR,
                        IARG_END
                    );
#else
                    INS_InsertPredicatedCall
                    (
                        ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                        IARG_MEMORYREAD2_EA,
                        IARG_MEMORYREAD_SIZE,
                        //IARG_UINT32, INS_IsPrefetch(ins),
                        IARG_END
                    );
#endif
                }
            }

            // Instrument Write accesses
            if( traceWrite && INS_IsMemoryWrite(ins) )
            {
                D2ECHO("Instrumenting Write in ins : " << INS_Disassemble(ins) );
#ifdef USEDEBUGENGINE2
                // Used in Debug of Engine 4
                INS_InsertPredicatedCall
                (
                    ins, IPOINT_BEFORE, (AFUNPTR)RecordWriteEngine4Debug,
                    IARG_MEMORYWRITE_EA,
                    IARG_MEMORYWRITE_SIZE,
                    //IARG_UINT32, INS_IsPrefetch(ins),
                    IARG_ADDRINT, (ADDRINT)(cstr),
                    IARG_INST_PTR,
                    IARG_END
                );
#else
                INS_InsertPredicatedCall
                (
                    ins, IPOINT_BEFORE, (AFUNPTR)WriteRecorder,
                    IARG_MEMORYWRITE_EA,
                    IARG_MEMORYWRITE_SIZE,
                    //IARG_UINT32, INS_IsPrefetch(ins),
                    IARG_END
                );
#endif
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
     * below instead of doing at analysis time in RoutineEntryRecorder(). This has
     * resulted in more functions in SeenFnames which may not be even involved in
     * communication. This, however, is not as such a problem as it will simply
     * clutter the output, and easily be cleaned in dot file etc.
     */

    IMG img = SEC_Img(RTN_Sec(rtn));

    if( IMG_IsMainExecutable(img) )
    {
        string rname = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_NAME_ONLY);
        D1ECHO ("rname (UNDECORATION_COMPLETE): " << rname );
        //string rname1 = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_COMPLETE);
        //D1ECHO ("rname (UNDECORATION_NAME_ONLY): " << rname1 );

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

            char* cstr = new char[ rname.size()+1 ];  // we need to dynamically allocate it so that it is available
                                                    // at analysis time
            strcpy(cstr, rname.c_str() );
            cstr[rname.size()] = '\0'; // null the last character TODO: is it required?
            D1ECHO ("Instrumenting Routine : " << cstr );

            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RoutineEntryRecorder,
                           IARG_ADDRINT, (ADDRINT)(cstr),
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
    PrintShadowMap();
#if (DEBUG>0)
    // Print Symbol Table to output file
    symTable.Print();
#endif

    switch( Engine )
    {
    case 1:
        if(TrackTasks == false)
        {
            ofstream rfout;
            OpenOutFile("recursivefunctions.dat", rfout);
            set<string>::iterator it1;
            for( it1=recursiveFunctions.begin(); it1 !=recursiveFunctions.end(); ++it1 )
            {
                rfout << *it1 << "\n";
            }
            rfout.close();
        }
        PrintMemAccesses();
        ComMatrix.PrintDot();
        ComMatrix.PrintMatrix();
        // callgraph.Print();
        callgraph.PrintText();
        callgraph.PrintJson();
        ComMatrix.PrintDependenceMatrix();
        break;
    case 2:
        if(TrackLoopDepend)
        {
            ofstream ilout;
            OpenOutFile("independentloopnames.dat", ilout);
            set<string>::iterator it1;
            set<string>::iterator it2;
            for( it1=independentLoopExecutions.begin(); it1 !=independentLoopExecutions.end(); ++it1 )
            {
                it2 = dependentLoopExecutions.find(*it1);
                if( it2 == dependentLoopExecutions.end() )
                {
                    ilout << *it1 << " ";
                    ECHO("Iterations of loop " << *it1 << " are independent");
                }
            }
            for( it1=dependentLoopExecutions.begin(); it1 !=dependentLoopExecutions.end(); ++it1 )
            {
                ECHO("Iterations of loop " << *it1 << " are dependent");
            }
            ilout.close();
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

    // PrintInstrCount();
    PrintInstrPercents();
    Locations.Print();
    tgout.close();
    traceout.close();
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
    SetCurrDir();
    PrintCurrDir();

    PIN_InitSymbols();
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if( PIN_Init(argc, argv) )
    {
        Usage();
        Die();
    }

    // Read locations available in locations.dat
    Locations.InitFromFile();

    Engine = KnobEngine.Value();

    TrackObjects = KnobTrackObjects.Value();
    RecordAllAllocations=KnobRecordAllAllocations.Value();
    FlushCalls=KnobFlushCalls.Value();
    FlushCallsLimit=KnobFlushCallsLimit.Value();
    ShowUnknown = KnobShowUnknown.Value();
    TrackStartStop = KnobTrackStartStop.Value();
    TrackZones = KnobTrackZones.Value();
    TrackLoopDepend = KnobTrackLoopDepend.Value();
    TrackTasks=KnobTrackTasks.Value();
    Threshold = KnobThreshold.Value();
    SelectedLoopNo = KnobSelectedLoopNo.Value();

    // TODO may be this can be pushed in constructor of symTable
    // furthermore, unknownObj can also be pushed!!!
    // Insert Unknown Ftn as first symbol
    symTable.InsertFunction(UnknownFtn, GlobalID++, 0); // 0 for unknown location index

    // Push the first ftn as UNKNOWN
    // The name can be adjusted from globals.h
    CallStack.Push(FuncName2ID[UnknownFtn]);
    CallSiteStack.Push(0); // 0 for unknown location index

    if( KnobReadStaticObjects.Value() )
    {
        symTable.InsertStaticSymbols(argc, argv);
    }

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

#if (DEBUG>0)
    ECHO("Printing Initial Symbol Table ...");
    symTable.Print();
#endif

    ECHO("Selecting Analysis Engine ...");
    if(TrackTasks)
        Engine=1;
    if(TrackLoopDepend)
        Engine=2;

    SelectAnalysisEngine();

    if( TrackStartStop || TrackLoopDepend)
    {
        // start dummy so that actual profiling starts when start is seen
        DoTrace=false;
    }
    else
    {
        // else start profiling right away
        DoTrace=true;
    }

    if ( Engine == 3 )
    {
        OpenOutFile(KnobPerCallFile.Value(), pcout);
    }

    // open the file to store taskgraph
    OpenOutFile("taskgraph.dat", tgout);

    // Open the output file to store traces
    OpenOutFile("traces.dat", traceout);
    traceout << "InsNumber AccessType AccessSize AccessAddr AccessValue" << endl;

    if( TrackTasks || TrackLoopDepend )
    {
        RoutineEntryRecorder = RoutineEntryRecorder2;
        RoutineExitRecorder = RoutineExitRecorder2;
    }
    else
    {
        RoutineEntryRecorder = RoutineEntryRecorder1;
        RoutineExitRecorder = RoutineExitRecorder1;
    }
//     RoutineEntryRecorder = RoutineEntryRecorder2;
//     RoutineExitRecorder = RoutineExitRecorder2;

    // Register function for Image-level instrumentation
    IMG_AddInstrumentFunction(InstrumentImages, 0);

    // Register function for Routine-level instrumentation
    RTN_AddInstrumentFunction(InstrumentRoutines, 0);

    // Register function for Trace-level instrumentation
    TRACE_AddInstrumentFunction(InstrumentTraces, 0);

#ifdef GENRATE_TRACES
    // Register function for Instruction-level instrumentation (used only for
    // counting instructions for generating traces)
    INS_AddInstrumentFunction(Instruction, 0);
#endif

    // Register function to be called when the application exits
    PIN_AddFiniFunction(TheEnd, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
