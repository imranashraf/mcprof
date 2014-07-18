#include "globals.h"
#include "shadow.h"
#include "commatrix.h"
#include "pintrace.h"
#include "pin.H"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    SetupPin(argc,argv);

    //RecordWrite(2,4,2);
    //RecordRead(3,4,2);

    PrintCommunication();

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
