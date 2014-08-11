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
//     cin.get();
#endif

//     Objects table;
//     table.Insert(Object(800, 4, 35, "testing.cpp"));
//     table.Insert(Object(800, 4, 36, "testing.cpp"));
//     table.Insert(Object(800, 4, 37, "testing.cpp"));
//     table.Print();
    
    SetupPin(argc,argv);

//     RecordWrite(2,4,4);
//     RecordRead(3,4,4);
//     PrintCommunication(cout, 5);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
