#include "globals.h"
#include "shadow.h"
#include "commatrix.h"

#include <iostream>

using namespace std;

int main()
{
    RecordWrite(2,4,2);
    RecordRead(3,4,2);

    PrintCommunication();
    
    return 0;
}