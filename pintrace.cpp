
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include "globals.h"
#include "pintrace.h"
#include "commatrix.h"
#include "shadow.h"
#include <iostream>
#include <fstream>
#include <stack>
#include <set>
#include <map>

/* ================================================================== */
// Global variables
/* ================================================================== */
std::ofstream dotout;
std::ofstream mout;
stack <string> CallStack;
map <string,UINT16> NametoADD;
map <UINT16,string> ADDtoName;
set<string> SeenFname;
UINT16 GlobalFunctionNo=0;


/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobMatrixFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "m", "matrix.out", "specify file name for matrix output");

KNOB<string> KnobDotFile(KNOB_MODE_WRITEONCE,  "pintool",
                         "d", "communication.dot",
                         "specify file name for output in dot");

KNOB<BOOL> KnobMainExecutableOnly(KNOB_MODE_WRITEONCE, "pintool",
                                  "MainExecutableOnly","1",
                                  "Trace functions that are contained only in the\
                                  executable image");


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
//     string ftnName("NA");
//     if( !CallStack.empty() )
//         ftnName = CallStack.top();

//     DECHO( VARS3(ftnName , NametoADD[ftnName], ip) <<" R "<<VARS2(addr, refSize) );
//     RecordRead( NametoADD[ftnName], (uptr)addr, refSize);
    RecordRead( NametoADD[CallStack.top()], (uptr)addr, refSize);
}

// Record a memory write
VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 refSize)
{
//     string ftnName("NA");
//     if( !CallStack.empty() )
//         ftnName = CallStack.top();

//     DECHO( VARS3(ftnName , NametoADD[ftnName], ip) <<" W "<<VARS2(addr, refSize) );
//     RecordWrite(NametoADD[ftnName], (uptr)addr,refSize);
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

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
            UINT32 memOperands = INS_MemoryOperandCount(ins);
            for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
                size_t refSize = INS_MemoryOperandSize(ins, memOp);
//                 bool isStack = INS_IsStackRead(ins);
//                 if(!isStack) return;

                if (INS_MemoryOperandIsRead(ins, memOp)) {
                    INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, memOp,
                        IARG_UINT32, refSize,
                        IARG_END);
                }

                if (INS_MemoryOperandIsWritten(ins, memOp)) {
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
#ifdef WIN32
        !name.compare("GetPdbDll") ||
        !name.compare("DebuggerRuntime") ||
        !name.compare("atexit") ||
        !name.compare("failwithmessage") ||
        !name.compare("pre_c_init") ||
        !name.compare("pre_cpp_init") ||
        !name.compare("mainCRTStartup") ||
        !name.compare("NtCurrentTeb") ||
        !name.compare("check_managed_app") ||
        !name.compare("DebuggerKnownHandle") ||
        !name.compare("DebuggerProbe") ||
        !name.compare("failwithmessage") ||
        !name.compare("unnamedImageEntryPoint"
#else
        !name.compare(".plt") ||
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
//     if( !ValidFtnName(rname) )
//         return;
    DECHO ("Entring Routine : " << rname);
    CallStack.push(rname);
}


VOID RecordRoutineExit(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string rname = PIN_UndecorateSymbolName(rtnName,
                                   UNDECORATION_NAME_ONLY);

    if(!(CallStack.empty()) && (CallStack.top() == rname)) {
        DECHO("Leaving Routine : " << rname);
        CallStack.pop();
//     } else if (!(CallStack.empty()) ) {
//         DECHO("Not Leaving Routine : "<< VAR(rname)
//             << VAR(CallStack.top()));
//     } else {
//         DECHO("Not Leaving Routine as CallStack empty without : "
//             << VAR(rname));
    }

}

// IMG instrumentation routine - called once per image upon image load
VOID Image_cb(IMG img, VOID * v)
{
    // For simplicity, instrument only the main image.
    // This can be extended to any other image of course.
    string img_name = IMG_Name(img);
    if (IMG_IsMainExecutable(img) == false &&
    KnobMainExecutableOnly.Value() == true) {
        DECHO("Skipping Image "<< img_name<< " as it is not main executable");
        return;
    } else {
        DECHO("Instrumenting "<<img_name<<" as it is the Main executable ");
    }

    // Traverse the sections of the image.
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {

        // For each section, process all RTNs.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {

            // Many RTN APIs require that the RTN be opened first.
            RTN_Open(rtn);
            string rname = PIN_UndecorateSymbolName( RTN_Name(rtn),
                                   UNDECORATION_NAME_ONLY);

            if( ValidFtnName(rname) ) {
                DECHO ("Instrumenting a valid routine" << rname );

                if(!SeenFname.count(rname)) { // First time seeing this function name
                    SeenFname.insert(rname);  // mark this function name as seen
                    GlobalFunctionNo++;      // create a Function Number for this function
                    NametoADD[rname]=GlobalFunctionNo;   // create String -> Number binding
                    ADDtoName[GlobalFunctionNo]=rname;   // create Number -> String binding
                }

                RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RecordRoutineEntry,
                            IARG_INST_PTR ,IARG_END);

                // Traverse all instructions
                for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
                    UINT32 memOperands = INS_MemoryOperandCount(ins);
                    for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
                        size_t refSize = INS_MemoryOperandSize(ins, memOp);
    //                 bool isStack = INS_IsStackRead(ins);
    //                 if(!isStack) return;

                        if (INS_MemoryOperandIsRead(ins, memOp)) {
                            INS_InsertPredicatedCall(
                                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                                IARG_INST_PTR,
                                IARG_MEMORYOP_EA, memOp,
                                IARG_UINT32, refSize,
                                IARG_END);
                        }

                        if (INS_MemoryOperandIsWritten(ins, memOp)) {
                            INS_InsertPredicatedCall(
                                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                                IARG_INST_PTR,
                                IARG_MEMORYOP_EA, memOp,
                                IARG_UINT32, refSize,
                                IARG_END);
                        }
                    }

                    if (INS_IsRet(ins)) {
                        INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                                                (AFUNPTR)RecordRoutineExit,
                                                IARG_INST_PTR, IARG_END);
                    }
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
VOID Fini(INT32 code, VOID *v)
{
//     PrintCommunication(cout, 5);
    PrintMatrix(mout, ADDtoName, GlobalFunctionNo);
    PrintCommunicationDot(dotout, ADDtoName, GlobalFunctionNo);
    dotout.close();
    mout.close();
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
    if( PIN_Init(argc,argv) ) {
        Usage();
        return;
    }

    string dfName = KnobDotFile.Value();
    if (!dfName.empty()) {
        dotout.open(dfName.c_str(), std::ios::out);
        if(dotout.fail()) {
            ECHO("Error Opening dot file");
            return;
        }
    } else {
        ECHO("Specify a non empty dot file name");
        return;
    }

    string mfName = KnobMatrixFile.Value();
    if (!mfName.empty()) {
        mout.open(mfName.c_str(), std::ios::out);
        if(mout.fail()) {
            ECHO("Error Opening matrix file");
            return;
        }
    } else {
        ECHO("Specify a non empty matrix file name");
        return;
    }

    string fname("UNKNOWN");
    SeenFname.insert(fname);             // Add UNKNOWN as the first function name
    NametoADD[fname]=GlobalFunctionNo;   // create the string -> Number binding
    ADDtoName[GlobalFunctionNo]=fname;   // create the Number -> String binding

    //TRACE_AddInstrumentFunction(Trace_cb, 0);
    //RTN_AddInstrumentFunction(RecordRoutineEntry,0);
    IMG_AddInstrumentFunction(Image_cb, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
