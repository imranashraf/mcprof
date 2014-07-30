#include "globals.h"
#include "shadow.h"
#include "commatrix.h"
#include "pintrace.h"
#include "pin.H"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
#if (DEBUG>0)
    PrintShadowMap();
//     cin.get();
#endif

    SetupPin(argc,argv);

//     RecordWrite(2,4,4);
//     RecordRead(3,4,4);
//     PrintCommunication(cout, 5);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
