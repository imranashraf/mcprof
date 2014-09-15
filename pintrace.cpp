
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include "globals.h"
#include "symbols.h"
#include "pintrace.h"
#include "commatrix.h"
#include "shadow.h"
#include "callstack.h"
#include "engine1.h"
#include "engine2.h"
#include "engine3.h"
#include "engine4.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <set>
#include <map>
#include <deque>
#include <algorithm>

/* ================================================================== */
// Global variables
/* ================================================================== */
extern map <string,IDNoType> FuncName2ID;
// extern map <u32,IDNoType> LocIndex2ID;
extern Symbols symTable;
extern Matrix2D ComMatrix;

std::ofstream dotout;
std::ofstream mout;
CallStackType CallStack;

void (*WriteRecorder)(uptr, u32);
void (*ReadRecorder)(uptr, u32);

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobMatrixFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "MatFile", "matrix.out",
                            "specify file name for matrix output");

KNOB<string> KnobDotFile(KNOB_MODE_WRITEONCE,  "pintool",
                         "DotFile", "communication.dot",
                         "specify file name for output in dot");

KNOB<BOOL> KnobMainExecutableOnly(KNOB_MODE_WRITEONCE, "pintool",
                                  "MainExecOnly","1",
                                  "Trace functions that are contained only in the\
                          executable image");

KNOB<BOOL> KnobStackAccess(KNOB_MODE_WRITEONCE, "pintool",
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

void SelectAnalysisEngine()
{
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
    case 4:
        ReadRecorder = RecordReadEngine4;
        WriteRecorder = RecordWriteEngine4;
        break;
    default:
        ECHO("Specify a valid Engine number to be used");
        Die();
        break;
    }
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

BOOL ValidFtnName(string name)
{ 
    // TODO name.compare > 0 or something like this ... or is it bool? (wrap_)
    return
        !(
            name.c_str()[0]=='_' ||
            name.c_str()[0]=='?' ||
            !name.compare("atexit") ||
//             !name.compare("wrap_") ||
            !name.compare("wrap_malloc") ||
            !name.compare("wrap_calloc") ||
            !name.compare("wrap_realloc") ||
            !name.compare("wrap_free") ||
            !name.compare("wrap_strdup") ||
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

VOID RecordRoutineEntry(CHAR* rname)
{
    D1ECHO ("Entering Routine : " << rname );
    CallStack.Push(FuncName2ID[rname]);

    // In engine 4, to save time, the curr call is selected only at func entry,
    // so that it does not need to be determined on each access
    if (KnobEngine.Value() == 4)
    {
        D1ECHO ("Setting Current Call for : " << rname );
        string str(rname);
        SetCurrCall( str );
    }
}


VOID RecordRoutineExit(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

    if(!(CallStack.Empty()) && ( CallStack.Top() == FuncName2ID[rname] ) )
    {
        D1ECHO("Leaving Routine : " << rname 
            << " Popping call stack top is " << symTable.GetSymName( CallStack.Top() ) );
        CallStack.Pop();
// #if (DEBUG>0)
    }
    else if (!(CallStack.Empty()) )
    {
        D1ECHO("Not Leaving Routine : "<< VAR(rname)
            << " call stack top is " << symTable.GetSymName( CallStack.Top() ) );
    }
    else
    {
        D1ECHO("Not Leaving Routine : " << VAR(rname) 
            << " as CallStack empty" );
// #endif
    }

}

IDNoType currID;
u32 currLocIndex;
u32 currSize;
uptr currStartAddress;

void SetIDOfCurrCall(u32 locIndex, u16 id)
{
    D2ECHO("setting last function call id/locIndex");
    currID = id;
    currLocIndex = locIndex; // TODO update locIndex (not really needed now)
}

// This is used both for malloc and calloc
VOID MallocBefore(u32 size)
{
    D2ECHO("setting malloc/calloc size " << size );
    currSize = size;
}

// This is used both for malloc and calloc
VOID MallocAfter(uptr addr)
{
    D2ECHO("setting malloc/calloc start address " << ADDR(addr) );
    currStartAddress = addr;
    symTable.InsertMallocCalloc(currID, currStartAddress, currLocIndex, currSize);
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
    uptr prevAddr = currStartAddress;
    //TODO change "0" to nullptr
    if(prevAddr == 0) //realloc behaves like malloc, supplied null address as argument
    {
        currStartAddress = addr;
        symTable.InsertMallocCalloc(currID, currStartAddress, currLocIndex, currSize);
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
        symTable.UpdateRealloc(currID, currStartAddress, currLocIndex, currSize);
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
    uptr srcAddr = currStartAddress;
    currStartAddress = dstAddr;

    IDNoType oldid = GetObjectID(srcAddr);
    u32 currSize = symTable.GetSymSize(oldid);

    symTable.InsertMallocCalloc(currID, currStartAddress, currLocIndex, currSize);
}

// IMG instrumentation routine - called once per image upon image load
VOID InstrumentImages(IMG img, VOID * v)
{
    string imgname = IMG_Name(img);

    // For simplicity, instrument only the main image.
    // This can be extended to any other image of course.
    if (IMG_IsMainExecutable(img) == false &&
            KnobMainExecutableOnly.Value() == true)
    {
        ECHO("Skipping Image "<< imgname<< " as it is not main executable");
        return;
    }
    else
    {
        ECHO("Instrumenting "<<imgname<<" as it is the Main executable ");
    }

    // we should instrument malloc/free only when tracking objects !
    if ( KnobTrackObjects.Value() )
    {
        // instrument libc for malloc, free etc
        D1ECHO("Instrumenting "<<imgname<<" for (re)(c)(m)alloc/free routines etc ");

        //  Find the malloc() function.
        RTN mallocRtn = RTN_FindByName(img, MALLOC.c_str() );
        if (RTN_Valid(mallocRtn))
        {
            RTN_Open(mallocRtn);

            // Instrument malloc() to print the input argument value and the return value.
            RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)MallocBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(mallocRtn);
        }

        RTN callocRtn = RTN_FindByName(img, CALLOC.c_str() );
        if (RTN_Valid(callocRtn))
        {
            RTN_Open(callocRtn);

            // Instrument calloc() to print the input argument value and the return value.
            // for now  using same callback ftns as for malloc
            RTN_InsertCall(callocRtn, IPOINT_BEFORE, (AFUNPTR)MallocBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(callocRtn);
        }

        RTN reallocRtn = RTN_FindByName(img, REALLOC.c_str() );
        if (RTN_Valid(reallocRtn))
        {
            RTN_Open(reallocRtn);

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

            // Instrument strdup() to print the input argument value and the return value.
            RTN_InsertCall(strdupRtn, IPOINT_BEFORE, (AFUNPTR)StrdupBefore,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_InsertCall(strdupRtn, IPOINT_AFTER, (AFUNPTR)StrdupAfter,
                           IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(strdupRtn);
        }

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
                //D2ECHO("disassembeled ins = " << INS_Disassemble(ins) );

                if( KnobTrackObjects.Value() && INS_IsDirectBranchOrCall(ins) ) // or should it be procedure call?
                {
                    ADDRINT target = INS_DirectBranchOrCallTargetAddress(ins);
                    string tname = Target2RtnName(target);
                    if( (tname == MALLOC) || (tname == CALLOC) ||
                        (tname == REALLOC) || (tname == STRDUP) )
                    {
                        string filename("");    // This will hold the source file name.
                        INT32 line = 0;     // This will hold the line number within the file.
                        PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &filename);

                        // Do not instrument the calls inside wrappers, so skip malloc_wrap.c file
                        if(filename.find("malloc_wrap.c") == string::npos ) // not found
                        {
                            // TODO may be rename locations to callsites
                            u32 locIndex = Locations.Insert( Location(line, filename) );
                            D1ECHO("Instrumenting library call for (re)(c)(m)alloc/free call at "
                                    << filename <<":"<< line);

                            // Get a new id for this location
                            IDNoType id=GlobalID++;

                            INS_InsertCall
                            (
                                ins,
                                IPOINT_BEFORE,
                                AFUNPTR(SetIDOfCurrCall),
                                IARG_UINT32, locIndex,
                                IARG_UINT32, id,
                                IARG_END
                            );
                        }
                    }
                }

                if (INS_IsRet(ins))
                {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                             (AFUNPTR)RecordRoutineExit,
                                             IARG_INST_PTR, IARG_END);
                }
            }
            RTN_Close(rtn); // Don't forget to close the RTN once you're done.
        }
    }
}

/*
VOID InstrumentInstructions(INS ins, VOID* v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        size_t refSize = INS_MemoryOperandSize(ins, memOp);
        if ( //KnobStackAccess.Value() && INS_IsStackRead(ins) && 
            INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                                        IARG_MEMORYOP_EA, memOp,
                                        IARG_UINT32, refSize,
                                        IARG_END);
        }
        
        if ( //KnobStackAccess.Value() && INS_IsStackWrite(ins) && 
            INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)WriteRecorder,
                                        IARG_MEMORYOP_EA, memOp,
                                        IARG_UINT32, refSize,
                                        IARG_END);
        }
    }
}
*/

/*
// without stack
VOID InstrumentTraces(TRACE trace, VOID *v)
{
    for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl=BBL_Next(bbl))
    {
        for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins=INS_Next(ins))
        {
            UINT32 memoryOperands = INS_MemoryOperandCount(ins);
            
            for (UINT32 memOp = 0; memOp < memoryOperands; memOp++)
            {
                UINT32 refSize = INS_MemoryOperandSize(ins, memOp);
                
                // Note that if the operand is both read and written,
                // we record it once for each.
                if (INS_MemoryOperandIsRead(ins, memOp))
                {
                    INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)ReadRecorder,
                                             IARG_MEMORYOP_EA, memOp,
                                             IARG_UINT32, refSize,
                                             IARG_END);
                }
                
                if (INS_MemoryOperandIsWritten(ins, memOp))
                {
                    INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)WriteRecorder,
                                             IARG_MEMORYOP_EA, memOp,
                                             IARG_UINT32, refSize,
                                             IARG_END);
                }
            }
        }
    }
}
*/

VOID InstrumentTraces(TRACE trace, VOID *v)
{
    for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl=BBL_Next(bbl))
    {
        for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins=INS_Next(ins))
        {
            
            if (INS_IsMemoryRead(ins) && !(KnobStackAccess.Value() && INS_IsStackRead(ins)) )
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
            
            if (INS_HasMemoryRead2(ins))
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
            
            if (INS_IsMemoryWrite(ins) && !(KnobStackAccess.Value() && INS_IsStackWrite(ins)))
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
    }
}


VOID InstrumentRoutines(RTN rtn, VOID *v)
{
    /*
     * The following function recording can be done at the instrumentation time as below
     * instead of doing at analysis time in RecordRoutineEntry(). This will result
     * in more functions in SeenFnames which may not be even involved in communication.
     * This, however, is not as such a problem as it will simply clutter the output.
     */
    IMG img = SEC_Img(RTN_Sec(rtn));
    //imagename = IMG_Name(img).c_str());
    
    if( IMG_IsMainExecutable(img) )
    {
        string rname = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_NAME_ONLY);

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
        OpenOutFile(KnobDotFile.Value(), dotout);
        OpenOutFile(KnobMatrixFile.Value(), mout);
        ComMatrix.PrintMatrix(mout);
        ComMatrix.PrintDot(dotout);
        mout.close();
        dotout.close();
        break;
    case 3:
        OpenOutFile(KnobDotFile.Value(), dotout);
        ComMatrix.PrintDot(dotout);
        dotout.close();
        break;
    case 4:
        PrintAllCalls();
        break;
    default:
        ECHO("Specify a valid Engine number to be used");
        Die();
        break;
    }

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
        symTable.InitFromFtnFile();
    }

    if(KnobSelectObjects.Value() )
    {
        D1ECHO("Initialize objects from Selected Objects file ...");
        symTable.InitFromObjFile();
    }

#if (DEBUG>0)
    ECHO("Printing Initial Symbol Table ...");
    symTable.Print();
#endif

    D1ECHO("Selecting Analysis Engine ...");
    SelectAnalysisEngine();

    // Register function for Image-level instrumentation
    IMG_AddInstrumentFunction(InstrumentImages, 0);
    // Register function for Routine-level instrumentation
    RTN_AddInstrumentFunction(InstrumentRoutines, 0);
    // Register function for Trace-level instrumentation
    TRACE_AddInstrumentFunction(InstrumentTraces, 0);
    // Register function for Instruction-level instrumentation
//     INS_AddInstrumentFunction(InstrumentInstructions, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(TheEnd, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
