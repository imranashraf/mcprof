
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include "pin.H"
#include "globals.h"
#include "pintrace.h"
#include <iostream>
#include <fstream>

/* ================================================================== */
// Global variables
/* ================================================================== */
std::ofstream fout;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "o", "memtrace.out", "specify file name for output");

KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool",
                       "count", "1", "count instructions, basic blocks and threads in the application");


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
    fout << ip << " R " <<addr<< " " << refSize << endl;
//     RecordRead(1,addr,refSize);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 refSize)
{
    fout << ip << " W " <<addr<< " " << refSize << endl;
//     RecordWrite(ftnNo,addr,refSize);
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

void Image_cb(IMG img, void *v)
{
    string img_name = IMG_Name(img);
    cerr << " Image = "<<img_name<<endl;
    
//     if (img_name.find("pintest_so.so") == string::npos) return;

    // Don't instrument these images
    if (img_name.find("/libc") != string::npos){
        cerr << " Skipping this image "<< img_name.find("/libc") <<endl;
        return;
    }

    if ( img_name.find("/usr/lib/") != 0 ) {
        cerr << " Skipping this image "<< img_name.find("/usr/lib/") << endl;
        return;
    }
    
    if ( img_name.find("/lib/") != 0){
        cerr << " Skipping this image "<< img_name.find("/lib/") << endl;
        return;
    }

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            string rtn_name = RTN_Name(rtn);
            cerr << " Rtn = "<<rtn_name<<endl;
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
    fout <<  "===============================================" << endl;
    fout <<  "          MCPROF Results:     " << endl;
    fout <<  "===============================================" << endl;
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

    if (KnobCount) {
        TRACE_AddInstrumentFunction(Trace_cb, 0);
        IMG_AddInstrumentFunction(Image_cb, 0);
        // Register function to be called when the application exits
        PIN_AddFiniFunction(Fini, 0);
    }
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
