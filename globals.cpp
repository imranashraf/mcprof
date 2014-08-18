#include "globals.h"
#include "pin.H"

using namespace std;

u16 UnknownID=0;
u16 GlobalID=UnknownID;
string UnknownFtn("UnknownFtn");
string UnknownObj("UnknownObj");

bool isEmpty(ifstream& fin)
{
    return fin.peek() == ifstream::traits_type::eof();
}

const string& Target2RtnName(uptr target)
{
    const string& name = RTN_FindNameByAddress(target);

    if (name == "")
        return *new string("[Unknown routine]");
    else
        return *new string(name);
}

const string& Target2LibName(uptr target)
{
    PIN_LockClient();
    const RTN rtn = RTN_FindByAddress(target);
    static const string _invalid_rtn("[Unknown image]");
    string name;

    if( RTN_Valid(rtn) )
        name = IMG_Name(SEC_Img(RTN_Sec(rtn)));
    else
        name = _invalid_rtn;

    PIN_UnlockClient();
    return *new string(name);
}

inline bool IsPowerOfTwo(uptr x)
{
    return (x & (x - 1)) == 0;
}

inline uptr RoundUpTo(uptr size, uptr boundary)
{
    CHECK(IsPowerOfTwo(boundary));
    return (size + boundary - 1) & ~(boundary - 1);
}

inline uptr RoundDownTo(uptr x, uptr boundary)
{
    return x & ~(boundary - 1);
}

inline bool IsAligned(uptr a, uptr alignment)
{
    return (a & (alignment - 1)) == 0;
}
