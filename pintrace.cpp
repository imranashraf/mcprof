
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include "globals.h"
#include "selectinstr.h"
#include "pintrace.h"
#include "commatrix.h"
#include "shadow.h"
#include "objects.h"
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
std::ofstream dotout;
std::ofstream mout;
stack <string> CallStack;
map <string,UINT16> NametoADD;
map <UINT16,string> ADDtoName;
UINT16 GlobalFunctionNo=0;
string UnKnownFtn("UNKNOWN");
FtnList SeenFnames;
Objects objTable;
Object objLast;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobMatrixFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "m", "matrix.out", "specify file name for matrix output");

KNOB<string> KnobDotFile(KNOB_MODE_WRITEONCE,  "pintool",
                         "d", "communication.dot",
                         "specify file name for output in dot");

KNOB<BOOL> KnobMainExecutableOnly(KNOB_MODE_WRITEONCE, "pintool",
                                  "MainExecOnly","1",
                                  "Trace functions that are contained only in the\
                          executable image");

KNOB<BOOL> KnobStackAccess(KNOB_MODE_WRITEONCE, "pintool",
                           "s","0", "Include Stack Accesses");

KNOB<BOOL> KnobSelecInstr(KNOB_MODE_WRITEONCE, "pintool",
                          "SelecInstr", "0",
                          "Instrument only the selected functions. \
                          The functions are listed in <SelecInstrList.txt> file");


/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    ECHO( "Memory and Data-Communication PROFiler.");
    ECHO( KNOB_BASE::StringKnobSummary() << endl);
    Die();
    return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

// Record a memory read
VOID RecordMemRead(VOID * ip, VOID * addr, UINT32 refSize)
{
    RecordRead( NametoADD[CallStack.top()], (uptr)addr, refSize);
}

// Record a memory write
VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 refSize)
{
    RecordWrite(NametoADD[CallStack.top()], (uptr)addr,refSize);
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 * Insert call to RecordMemRead/RecordMemWrite analysis routine before
 * every basic block of the trace.
 * This function is called every time a new trace is encountered.
 * @param[in]   trace    trace to be instrumented
 * @param[in]   v        value specified by the tool in the TRACE_AddInstrumentFunction
 *                       function call
 */
void Trace_cb(TRACE trace, void *v)
{
    RTN rtn = TRACE_Rtn(trace);
    if (!RTN_Valid(rtn)) return;
//     string rtn_name = RTN_Name(rtn);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            UINT32 memOperands = INS_MemoryOperandCount(ins);
            for (UINT32 memOp = 0; memOp < memOperands; memOp++)
            {
                size_t refSize = INS_MemoryOperandSize(ins, memOp);
//                 bool isStack = INS_IsStackRead(ins);
//                 if(!isStack) return;

                if (INS_MemoryOperandIsRead(ins, memOp))
                {
                    INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, memOp,
                        IARG_UINT32, refSize,
                        IARG_END);
                }

                if (INS_MemoryOperandIsWritten(ins, memOp))
                {
                    INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, memOp,
                        IARG_UINT32, refSize,
                        IARG_END);
                }
            }
        }
    }
}

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
#define RTNOPT 1
VOID RecordRoutineEntry(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
#if (RTNOPT==0)
    if( !ValidFtnName(rname) )
        return;

    SeenFnames.AddIfNotFound(rname);
#endif

    D1ECHO ("Entring Routine : " << rname << VAR(GlobalFunctionNo));
    CallStack.push(rname);
}


VOID RecordRoutineExit(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

    if(!(CallStack.empty()) && (CallStack.top() == rname))
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

// Names of malloc and free
string MALLOC("malloc");
string FREE("free");
string invalid("invalid_rtn");

const string *Target2String(ADDRINT target)
{
    string name = RTN_FindNameByAddress(target);
    if (name == "")
        return &invalid;
    else
        return new string(name);
}

const string& Target2RtnName(ADDRINT target)
{
    const string& name = RTN_FindNameByAddress(target);

    if (name == "")
        return *new string("[Unknown routine]");
    else
        return *new string(name);
}

const string& Target2LibName(ADDRINT target)
{
    PIN_LockClient();
    const RTN rtn = RTN_FindByAddress(target);
    static const string _invalid_rtn("[Unknown image]");
    string name;

    if( RTN_Valid(rtn) )
        name = IMG_Name(SEC_Img(RTN_Sec(rtn)));
    else
        name = _invalid_rtn;

    PIN_UnlockClient();
    return *new string(name);
}

void setLoc(int l, char* f)
{
    cout << "setting last function call location as " << f << ":" << l << endl;
    objLast.SetLineFile(l, f);
}

VOID Arg1Before(ADDRINT size)
{
    cout << " setting malloc size " << size << endl;
    objLast.SetSize(size);
}

VOID MallocAfter(ADDRINT addr)
{
    cout << " setting malloc start address " << ADDR(addr) << endl;
    objLast.SetAddr(addr);
    objTable.Insert(objLast);
}

// IMG instrumentation routine - called once per image upon image load
VOID Image_cb(IMG img, VOID * v)
{
    string imgname = IMG_Name(img);

    // instrument libc for malloc, free etc
    if ( imgname.find("libc") != string::npos )
    {
        ECHO("Instrumenting "<<imgname<<" for malloc, free etc ");

        // Instrument the malloc() and free() functions.  Print the input argument
        // of each malloc() or free(), and the return value of malloc().
        //
        //  Find the malloc() function.
        RTN mallocRtn = RTN_FindByName(img, MALLOC.c_str() );
        if (RTN_Valid(mallocRtn))
        {
            RTN_Open(mallocRtn);

            // Instrument malloc() to print the input argument value and the return value.
            RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
                        IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                        IARG_END);
            RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
                        IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            RTN_Close(mallocRtn);
        }

//         // Find the free() function.
//         RTN freeRtn = RTN_FindByName(img, FREE);
//         if (RTN_Valid(freeRtn))
//         {
//             RTN_Open(freeRtn);
//             // Instrument free() to print the input argument value.
//             RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
//                         IARG_ADDRINT, FREE,
//                         IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
//                         IARG_END);
//             RTN_Close(freeRtn);
//         }


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
            * instead of doing at analysis time in RecordRoutineEntry(). This will simply result
            * in more functions in SeenFnames which may not be even involved in communication,
            * which is not as such a problem as it will simply clutter the output.
            */
#if (RTNOPT==1)
            string rname = PIN_UndecorateSymbolName( RTN_Name(rtn), UNDECORATION_NAME_ONLY);

            if( KnobSelecInstr.Value() )
            {
                // If this valid function name is not in the selected instr. list
                if(!SeenFnames.Find(rname))
                {
                    D1ECHO ("Skipping Instrumentation of Routine : " << rname);
                    continue; // skip it
                }
            }
            else
            {
                if (!ValidFtnName(rname))
                {
                    D1ECHO ("Skipping Instrumentation of Routine : " << rname);
                    continue;
                }

                // First time seeing this valid function name, save it in the list
                SeenFnames.Add(rname);
            }
            D1ECHO ("Instrumenting Routine : " << rname);
#endif

            // Many RTN APIs require that the RTN be opened first.
            RTN_Open(rtn);

            // Rest (apart from .plt) of the valid routines are instrumented
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RecordRoutineEntry,
                           IARG_INST_PTR ,IARG_END);

            // Traverse all instructions
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                UINT32 memOperands = INS_MemoryOperandCount(ins);

                if( INS_IsCall(ins) ) // or should it be procedure call?
                {
                    string filename;    // This will hold the source file name.
                    INT32 line = 0;     // This will hold the line number within the file.
                    PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &filename);
                    INS_InsertCall(ins,
                        IPOINT_BEFORE,
                        AFUNPTR(setLoc),
                        IARG_UINT32, line,
                        IARG_PTR, filename.c_str(),
                        IARG_END);
                }

                bool isStack = INS_IsStackRead(ins) || INS_IsStackWrite(ins);
                if(!isStack || KnobStackAccess.Value())
                {
                    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
                    {
                        size_t refSize = INS_MemoryOperandSize(ins, memOp);
                        if (INS_MemoryOperandIsRead(ins, memOp))
                        {
                            INS_InsertPredicatedCall(
                                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                                IARG_INST_PTR,
                                IARG_MEMORYOP_EA, memOp,
                                IARG_UINT32, refSize,
                                IARG_END);
                        }

                        if (INS_MemoryOperandIsWritten(ins, memOp))
                        {
                            INS_InsertPredicatedCall(
                                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                                IARG_INST_PTR,
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
#if (DEBUG>0)
    PrintCommunication(cout, 7);
#endif
    PrintMatrix(mout, GlobalFunctionNo);
    PrintCommunicationDot(dotout, GlobalFunctionNo);
    dotout.close();
    mout.close();

    objTable.Print();
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
        return;
    }

    string dfName = KnobDotFile.Value();
    if (!dfName.empty())
    {
        dotout.open(dfName.c_str(), std::ios::out);
        if(dotout.fail())
        {
            ECHO("Error Opening dot file");
            return;
        }
    }
    else
    {
        ECHO("Specify a non empty dot file name");
        return;
    }

    string mfName = KnobMatrixFile.Value();
    if (!mfName.empty())
    {
        mout.open(mfName.c_str(), std::ios::out);
        if(mout.fail())
        {
            ECHO("Error Opening matrix file");
            return;
        }
    }
    else
    {
        ECHO("Specify a non empty matrix file name");
        return;
    }

    // Push the first ftn as UNKNOWN
    // The name can be adjusted from globals.h
    CallStack.push(UnKnownFtn);
    if(KnobSelecInstr.Value())
    {
        SeenFnames.InitFromFile();
    }

    //TRACE_AddInstrumentFunction(Trace_cb, 0);
    //RTN_AddInstrumentFunction(RecordRoutineEntry,0);
    IMG_AddInstrumentFunction(Image_cb, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(TheEnd, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */

// analysis routines for the below
// void ProcessDirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp)
// {
//     ECHO("Direct call: " << Target2RtnName(target));
// }
// 
// void ProcessIndirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp, ADDRINT arg, ADDRINT rip)
// {
//     string rtnName = Target2RtnName(target);
//     // apply undecoration ???
//     if( rtnName == MALLOC)
//         ECHO("Indirect call : " << rtnName << " " << arg << ADDR(rip) );
//     else if (rtnName == FREE)
//         ECHO("Indirect call : " << rtnName << " " << ADDR(arg));
// }
// 
// void ProcessStub(ADDRINT ip, ADDRINT target)
// {
//     ECHO("Instrumenting stub: " << Target2String(target));
//     ECHO("STUB: " << Target2RtnName(target) );
// }


// following was the instrumentation done for .plt section
//             // The .plt sec of main image is instrumented for calls to malloc
//             // and free. Furthermore, based on the logic of decoding the
//             // address of these routines in shared libraries, a dummy malloc
//             // should be called first. Suceeding mallocs will be detected and
//             // decoded properly.
//             if (rname == ".plt")
//             {
//                 ECHO("in .plt ");
//                 // Traverse all instructions
//                 for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
//                 {
//                     string  inscode = INS_Disassemble (ins);
//                     ECHO(VAR(inscode));
// 
//                     if( INS_IsBranchOrCall(ins) )
//                     {
//                         if( INS_IsDirectBranchOrCall(ins) )
//                         {
//                             ECHO("Instrumenting INS_IsDirectBranchOrCall");
//                             ADDRINT target = INS_DirectBranchOrCallTargetAddress(ins);
//                             string rtnName = RTN_FindNameByAddress(target);
//                             ECHO("Instrumenting "<< rtnName);
//                             string rname = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);
//                             INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
//                                                      (AFUNPTR)ProcessDirectCall,
//                                                      IARG_INST_PTR,
//                                                      IARG_ADDRINT, target,
//                                                      IARG_REG_VALUE, REG_STACK_PTR,
//                                                      IARG_END);
// 
//                         }
//                         else if( INS_IsIndirectBranchOrCall(ins) )
//                         {
//                             ECHO("Instrumenting INS_IsIndirectBranchOrCall");
//                             string filename;    // This will hold the source file name.
//                             INT32 line = 0;     // This will hold the line number within the file.
//                             PIN_GetSourceLocation(INS_Address(ins), NULL, &line, &filename);
//                             ECHO( "src info " << filename <<":"<<line);
//                             INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
//                                                      (AFUNPTR)ProcessIndirectCall,
//                                                      IARG_INST_PTR,
//                                                      IARG_BRANCH_TARGET_ADDR,
//                                                      IARG_REG_VALUE, REG_STACK_PTR,
//                                                      IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
//                                                      IARG_RETURN_IP,
//                                                      IARG_END);
//                         }
//                         else
//                         {
//                             ECHO("some other branch");
//                         }
//                     }
//                 }
//                 //we dont need to do any thing else for .plt
//                 RTN_Close(rtn); // so close the RTN once you're done.
//                 continue;       // and go back
//             }
