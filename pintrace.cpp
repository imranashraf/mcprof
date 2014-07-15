/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <iostream>
#include <fstream>

#include "pin.H"

/* ================================================================== */
// Global variables
/* ================================================================== */
// std::ostream * fout = &cerr;
std::ofstream fout;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
                            "o", "memtrace.out", "specify file name for MyPinTool output");

KNOB<BOOL>   KnobMemTrace(KNOB_MODE_WRITEONCE,  "pintool",
                       "trace", "1", "trace memory accesses in the application");
/* ===================================================================== */
// Utilities
/* ===================================================================== */

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This Pintool prints a trace of memory addresses"<<endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    
    return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr, UINT32 refSize)
{
    fout << ip << " " << addr << " R " << refSize << "\n"; 
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 refSize)
{
    fout << ip << " " << addr << " W " << refSize << "\n"; 
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        UINT32 refSize = INS_MemoryOperandSize(ins, memOp);

        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, refSize,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
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

/* ===================================================================== */
/* Fini                                                                  */
/* ===================================================================== */
/*!
 * The Fini procedure of the tool.
 * This function is called at the end of the application.
 */
VOID Fini(INT32 code, VOID *v)
{
    fout <<  "#eof"<<endl;
    fout.close();
}

/* ===================================================================== */
/* SetupPin                                                              */
/* ===================================================================== */
/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments,
 *                              including pin -t <toolname> -- ...
 */
void SetupPin(int argc, char *argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();
    
    if (PIN_Init(argc, argv)){
      Usage();
      return;
    }

    string fileName = KnobOutputFile.Value();
    if (!fileName.empty()) {
        fout.open(fileName.c_str()); 
        if(fout.fail()){
            cerr << "Error Opening File" << endl;
            return;
        }
    }
    
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
}
