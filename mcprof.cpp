#include "globals.h"
#include "shadow.h"
#include "commatrix.h"
#include "pintrace.h"
#include "symbols.h"
#include "pin.H"

#include <iostream>

using namespace std;

/* ================================================================== */
// Global variables
/* ================================================================== */

// Main Symbol Table
Symbols symTable;

// Matrix to hold communication
Matrix2D ComMatrix;

// some mappings for faster access
map <string,IDNoType> FuncName2ID;  // updated on the fly only for functions
map <u32,IDNoType> LocIndex2ID;

/* ================================================================== */
// Main
/* ================================================================== */

int main(int argc, char *argv[])
{
#if (DEBUG>0)
    PrintShadowMap();
#endif

    SetupPin(argc,argv);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
