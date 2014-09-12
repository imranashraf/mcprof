#include "globals.h"
#include "pin.H"

IDNoType GlobalID=UnknownID;

using namespace std;

bool isEmpty(ifstream& fin)
{
    return fin.peek() == ifstream::traits_type::eof();
}

void OpenInFile(const string& fileName, ifstream& fin)
{
    if (!fileName.empty())
    {
        fin.open(fileName.c_str());
        if ( fin.fail() )
        {
            ECHO("Can not open input file (" <<fileName.c_str() << "). Aborting...");
            Die();
        }
    }
    else
    {
        ECHO("Specify a non empty file name. Aborting ...");
        Die();
    }

    if(isEmpty(fin))
    {
        ECHO("Input file (" <<fileName.c_str()<<") is empty. Aborting...");
        Die();
    }
}

void OpenOutFile(const string& fileName, ofstream& fout)
{
    if (!fileName.empty())
    {
        fout.open(fileName.c_str());
        if ( fout.fail() )
        {
            ECHO("Can not open output file (" <<fileName.c_str() << "). Aborting ...");
            Die();
        }
    }
    else
    {
        ECHO("Specify a non empty file name. Aborting ...");
        Die();
    }
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
