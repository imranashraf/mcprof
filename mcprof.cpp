#include "globals.h"
#include "shadow.h"
#include "commatrix.h"
#include "pintrace.h"
#include "objects.h"
#include "pin.H"

#include <iostream>

using namespace std;

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
