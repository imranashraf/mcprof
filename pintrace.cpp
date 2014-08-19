
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
extern map <string,IDNoType> Name2ID;
extern map <IDNoType,string> ID2Name;
extern Symbols symTable;

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

KNOB<BOOL> KnobSelectFunctions(KNOB_MODE_WRITEONCE, "pintool",
                               "SelectFunctions", "0",
                               "Instrument only the selected functions. \
                                User provides functions in <SelectFunctions.txt> file");

KNOB<BOOL> KnobSelectObjects(KNOB_MODE_WRITEONCE, "pintool",
                             "SelectObjects", "1",
                             "Instrument only the selected objects. \
                              User provides objects in <SelectObjects.txt> file");

KNOB<UINT32> KnobEngine(KNOB_MODE_WRITEONCE,  "pintool",
                        "Engine", "2",
                        "specify engine to be used");

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
    return
        !(
//         name[0]=='_' ||
            name[0]=='?' ||
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

VOID RecordRoutineEntry(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

    D1ECHO ("Entring Routine : " << rname );
    CallStack.push(Name2ID[rname]);

    if (KnobEngine.Value() == 4)
        SetCurrCall(rname);
}


VOID RecordRoutineExit(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

    if(!(CallStack.empty()) && ( CallStack.top() == Name2ID[rname] ) )
    {
        D1ECHO("Leaving Routine : " << rname);
        CallStack.pop();
#if (DEBUG>0)
    }
    else if (!(CallStack.empty()) )
    {
        D1ECHO("Not Leaving Routine : "<< VAR(rname)
               << VAR(CallStack.top()));
    }
    else
    {
        D1ECHO("Not Leaving Routine as CallStack empty without : "
               << VAR(rname));
#endif
    }

}

Symbol newSymbol;
Symbol* currSymbol = &newSymbol; // TODO setting it to nullptr crashes

void SelectExistingSym(IDNoType id)
{
    D2ECHO("setting location index to an already available sym location");
    currSymbol = symTable.GetSymbolPtr(id); // pointing to an existing symbol
}

void SelectNewSym(u32 locidx)
{
    D2ECHO("setting last function call location to new sym location ");
    currSymbol = &newSymbol;    // pointing to new symbol
    currSymbol->SetLocIndex(locidx);    // so update location as well
}

VOID MallocBefore(u32 size)
{
    D2ECHO(" setting malloc size " << size );
    currSymbol->SetSize(size);
}

VOID MallocAfter(uptr addr)
{
    D2ECHO("setting malloc start address " << ADDR(addr) );
    currSymbol->SetStartAddr(addr);

    // If Selected Objects are not supplied, then insert objects to table
    // TODO try to optimize it away at instrumentation time
    if( !KnobSelectObjects.Value() )
    {
        symTable.InsertObject(*currSymbol);
    }
}

VOID FreeBefore(ADDRINT addr)
{
    if(addr != 0)
    {
        D2ECHO("removing object with start address " << ADDR(addr) );
//         symTable.Remove(addr);   // comment it to keep the object table
        // useful for debugging
    }
}

// IMG instrumentation routine - called once per image upon image load
VOID Image_cb(IMG img, VOID * v)
{
    string imgname = IMG_Name(img);

    // instrument libc for malloc, free etc
    if ( imgname.find("libc") != string::npos )
    {
        D1ECHO("Instrumenting "<<imgname<<" for malloc, free etc ");

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

        // no need to do any thing else for libc, so can return
        return;
    }

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

    // Traverse the sections of the image.
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {

        // For each section, process all RTNs.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            /*
            * The following function recording can be done at the instrumentation time as below
            * instead of doing at analysis time in RecordRoutineEntry(). This will result
            * in more functions in SeenFnames which may not be even involved in communication.
            * This, however, is not as such a problem as it will simply clutter the output.
            */

            string rname = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_NAME_ONLY);

            if (!ValidFtnName(rname))
            {
                D1ECHO ("Skipping Instrumentation of Invalid Routine : " << rname);
                continue;
            }

            if( KnobSelectFunctions.Value() )
            {
                // In Select Function mode, functions are added a priori from
                // the list file to symbol table. So, if a function is not
                // found in symbol table, it means it is not in select ftn list
                // so it should be skiped
                if( ! symTable.IsSeenFunctionName(rname) )
                {
                    D1ECHO ("Skipping Instrumentation of un-selected Routine : " << rname);
                    continue;
                }
            }
            else
            {
                // First time seeing this valid function name, save it in the list
                symTable.InsertFunction(rname);
            }

            D1ECHO ("Instrumenting Routine : " << rname);

            // Many RTN APIs require that the RTN be opened first.
            RTN_Open(rtn);

            // Rest (apart from .plt) of the valid routines are instrumented
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RecordRoutineEntry,
                           IARG_INST_PTR ,IARG_END);

            // Traverse all instructions
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                if( INS_IsCall(ins) ) // or should it be procedure call?
                {
                    ADDRINT target = INS_DirectBranchOrCallTargetAddress(ins);
                    string tname = Target2RtnName(target);
                    D2ECHO( "Calling " << tname );

                    if(tname == ".plt")
                    {
                        string filename("");    // This will hold the source file name.
                        INT32 line = 0;     // This will hold the line number within the file.
                        PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &filename);

                        if (KnobSelectObjects.Value() )
                        {
                            u32 symid = symTable.GetID(filename, line);
                            if ( symid != UnknownID )
                            {
                                D2ECHO("Instrumenting object (re)alloc/free call at "
                                       << filename <<":"<< line << " available in table");

                                INS_InsertCall
                                (
                                    ins,
                                    IPOINT_BEFORE,
                                    AFUNPTR(SelectExistingSym),
                                    IARG_UINT32, symid,
                                    IARG_END
                                );
                            }
                        }
                        else
                        {
                            u32 locIndex = Locations.Insert( Location(line, filename) );
                            D1ECHO("Instrumenting object (re)alloc/free call at "
                                   << filename <<":"<< line);
                            INS_InsertCall
                            (
                                ins,
                                IPOINT_BEFORE,
                                AFUNPTR(SelectNewSym),
                                IARG_UINT32, locIndex,
                                IARG_END
                            );
                        }
                    }
                }

                UINT32 memOperands = INS_MemoryOperandCount(ins);
                bool isStack = INS_IsStackRead(ins) || INS_IsStackWrite(ins);
                if(!isStack || KnobStackAccess.Value())
                {
                    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
                    {
                        size_t refSize = INS_MemoryOperandSize(ins, memOp);
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


/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID TheEnd(INT32 code, VOID *v)
{
    // Generate symbol names for un-named symbols

#if (DEBUG>0)
    symTable.Print();
    PrintCommunication(cout, 7);
#endif
    PrintMatrix(mout, GlobalID);
    mout.close();
    PrintCommunicationDot(dotout, GlobalID);
    dotout.close();

//     if (KnobEngine.Value() == 4)
//         PrintAllCalls(); // in engine 4 only

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

    string dfName = KnobDotFile.Value();
    if (!dfName.empty())
    {
        dotout.open(dfName.c_str(), std::ios::out);
        if(dotout.fail())
        {
            ECHO("Error Opening dot file");
            Die();
        }
    }
    else
    {
        ECHO("Specify a non empty dot file name");
        Die();
    }

    string mfName = KnobMatrixFile.Value();
    if (!mfName.empty())
    {
        mout.open(mfName.c_str(), std::ios::out);
        if(mout.fail())
        {
            ECHO("Error Opening matrix file");
            Die();
        }
    }
    else
    {
        ECHO("Specify a non empty matrix file name");
        Die();
    }

    // TODO may be this can be pushed in constructor of symTable
    // furthermore, unknownObj can also be pushed!!!
    // Insert Unknown Ftn as first symbol
    symTable.InsertFunction(UnknownFtn);

    // Push the first ftn as UNKNOWN
    // The name can be adjusted from globals.h
    CallStack.push(Name2ID[UnknownFtn]);

    if(KnobSelectFunctions.Value())
    {
        symTable.InitFromFtnFile();
    }

    if(KnobSelectObjects.Value() )
    {
        symTable.InitFromObjFile();
    }

    symTable.Print();

    SelectAnalysisEngine();

    //RTN_AddInstrumentFunction(RecordRoutineEntry,0);
    IMG_AddInstrumentFunction(Image_cb, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(TheEnd, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
