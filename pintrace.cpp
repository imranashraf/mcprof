
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
std::ofstream fout;
std::ofstream dotout;
stack <string> CallStack;
map <string,UINT16> NametoADD;
map <UINT16,string> ADDtoName;
set<string> SeenFname;
UINT16 GlobalFunctionNo=0;


/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "o", "memtrace.out", "specify file name for output");

KNOB<string> KnobDotFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "d", "communication.dot", "specify file name for output in dot");

KNOB<BOOL> KnobMainExecutableOnly(KNOB_MODE_WRITEONCE, "pintool",
    "MainExecutableOnly","1", "Trace functions that are contained only in the main executable image");


/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl <<
         "instructions, basic blocks and threads in the application." << endl << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr, UINT32 refSize)
{
    string ftnName("NA");
    if( !CallStack.empty() )
        ftnName = CallStack.top();

    //fout << ftnName << "(" << NametoADD[ftnName] << ") " << ip << " R " <<addr<< " " << refSize << endl;
    RecordRead( NametoADD[ftnName], (uptr)addr, refSize);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 refSize)
{
    string ftnName("NA");
    if( !CallStack.empty() )
        ftnName = CallStack.top();

    //fout << ftnName << "(" << NametoADD[ftnName] << ") " << ip << " W " <<addr<< " " << refSize << endl;
    RecordWrite(NametoADD[ftnName], (uptr)addr,refSize);
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 * Insert call to the CountBbl() analysis routine before every basic block
 * of the trace.
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
            for (UINT32 memOp = 0; memOp < memOperands; memOp++){
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


VOID RecordRoutineEntry(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string name = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

    if (
        name[0]=='_' ||
        name[0]=='?' ||
#ifdef WIN32
        !strcmp(name,"GetPdbDll") ||
        !strcmp(name,"DebuggerRuntime") ||
        !strcmp(name,"atexit") ||
        !strcmp(name,"failwithmessage") ||
        !strcmp(name,"pre_c_init") ||
        !strcmp(name,"pre_cpp_init") ||
        !strcmp(name,"mainCRTStartup") ||
        !strcmp(name,"NtCurrentTeb") ||
        !strcmp(name,"check_managed_app") ||
        !strcmp(name,"DebuggerKnownHandle") ||
        !strcmp(name,"DebuggerProbe") ||
        !strcmp(name,"failwithmessage") ||
        !strcmp(name,"unnamedImageEntryPoint"
#else
        !name.compare(".plt") ||
        !name.compare("call_gmon_start") ||
        !name.compare("register_tm_clones") ||
        !name.compare("deregister_tm_clones") ||
        !name.compare("frame_dummy")
#endif
        )
        return;

    if(!SeenFname.count(name)) { // this is the first time I see this function name
        SeenFname.insert(name);  // mark this function name as seen
        GlobalFunctionNo++;      // create a dummy Function Number for this function
        NametoADD[name]=GlobalFunctionNo;   // create the string -> Number binding
        ADDtoName[GlobalFunctionNo]=name;   // create the Number -> String binding
    }

    cout << "Entring Routine : "<< name << endl;
    CallStack.push(name);
}


VOID RecordRoutineExit(VOID *ip)
{
    string rtnName = RTN_FindNameByAddress((ADDRINT)ip);
    string demangledNameNoParams = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

    //if( !(CallStack.empty()) ) {
    if(!(CallStack.empty()) && (CallStack.top() == demangledNameNoParams)) {
        cout << " Leaving Routine : "<< demangledNameNoParams << endl;
        CallStack.pop();
    }
    else if (!(CallStack.empty()) ) {
        cout << " Not Leaving Routine : "<< demangledNameNoParams << endl;
        cout << " Return Stack Top: "<< CallStack.top() << endl;
    }
    else{
        cout << " Not Leaving Routine as CallStack empty without : "<< demangledNameNoParams << endl;
    }

 
}

// IMG instrumentation routine - called once per image upon image load
VOID Image_cb(IMG img, VOID * v)
{
    // For simplicity, instrument only the main image. This can be extended to any other image of course.
    string img_name = IMG_Name(img);
    if (IMG_IsMainExecutable(img) == false &&
        KnobMainExecutableOnly.Value() == true) {
        cout << " Skipping Image "<<img_name<< " as it is not main executable " << endl;
        return;
    }
    else {
        cout << " Instrumenting "<<img_name<< " as it is the Main executable " <<endl;
    }

    // To find all the instructions in the image, we traverse the sections of the image.
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {

        // For each section, process all RTNs.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {

            // Many RTN APIs require that the RTN be opened first.
            RTN_Open(rtn);

            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)RecordRoutineEntry,
                    IARG_INST_PTR ,IARG_END);

            // Call PIN_GetSourceLocation for all the instructions of the RTN.
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
                UINT32 memOperands = INS_MemoryOperandCount(ins);
                for (UINT32 memOp = 0; memOp < memOperands; memOp++){
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

                if (INS_IsRet(ins)){
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
VOID Fini(INT32 code, VOID *v)
{
    PrintCommunication();
    PrintCommunicationDot(dotout, ADDtoName, GlobalFunctionNo);

    fout <<  "===============================================" << endl;
    fout <<  "          The End     " << endl;
    fout <<  "===============================================" << endl;

    fout.close();
    dotout.close();
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

    string fileName = KnobOutputFile.Value();
    if (!fileName.empty()) {
        fout.open(fileName.c_str(), std::ios::out);
        if(fout.fail()){
            cerr << "Error Opening file"<<endl;
            return;
        }
    }
    else{
        cerr << "Specify a non empty file name"<<endl;
        return;
    }

    string dfileName = KnobDotFile.Value();
    if (!dfileName.empty()) {
        dotout.open(dfileName.c_str(), std::ios::out);
        if(dotout.fail()){
            cerr << "Error Opening dot file"<<endl;
            return;
        }
    }
    else{
        cerr << "Specify a non empty dot file name"<<endl;
        return;
    }


    //TRACE_AddInstrumentFunction(Trace_cb, 0);
    //RTN_AddInstrumentFunction(RecordRoutineEntry,0);
    IMG_AddInstrumentFunction(Image_cb, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
