#include "globals.h"
#include "pin.H"

IDNoType GlobalID=UnknownID;

using namespace std;

void PrintLogo()
{
    cerr << " __       __   ______   _______   _______    ______   ________ "<<endl;
    cerr << "/  \\     /  | /      \\ /       \\ /       \\  /      \\ /        |"<<endl;
    cerr << "$$  \\   /$$ |/$$$$$$  |$$$$$$$  |$$$$$$$  |/$$$$$$  |$$$$$$$$/ "<<endl;
    cerr << "$$$  \\ /$$$ |$$ |  $$/ $$ |__$$ |$$ |__$$ |$$ |  $$ |$$ |__    "<<endl;
    cerr << "$$$$  /$$$$ |$$ |      $$    $$/ $$    $$< $$ |  $$ |$$    |   "<<endl;
    cerr << "$$ $$ $$/$$ |$$ |   __ $$$$$$$/  $$$$$$$  |$$ |  $$ |$$$$$/    "<<endl;
    cerr << "$$ |$$$/ $$ |$$ \\__/  |$$ |      $$ |  $$ |$$ \\__$$ |$$ |      "<<endl;
    cerr << "$$ | $/  $$ |$$    $$/ $$ |      $$ |  $$ |$$    $$/ $$ |      "<<endl;
    cerr << "$$/      $$/  $$$$$$/  $$/       $$/   $$/  $$$$$$/  $$/       "<<endl;
    cerr << "                                                               "<<endl;

//     cerr << " __  __  ___  ____  ____  _____  ____  " <<endl
//          <<   "(  \\/  )/ __)(  _ \\(  _ \\(  _  )( ___) " <<endl
//          <<   " )    (( (__  )___/ )   / )(_)(  )__)  " <<endl
//          <<   "(_/\\/\\_)\\___)(__)  (_)\\_)(_____)(__)   " <<endl;

}

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
        fout.open(fileName.c_str(), ios::binary);
        if ( fout.fail() )
        {
            ECHO("Can not open output file (" << fileName.c_str() << "). Aborting ...");
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

const std::string& humanReadableByteCount(u64 bytes, bool si)
{
    string hBytes;
    char cBytes[100];
    u16 unit = si ? 1000 : 1024;
    string punits1 = (si ? "kMGTPE" : "KMGTPE");
    char punits2 = (si ? '\0' : 'i');
    char pre[3];

    if (bytes < unit)
        hBytes = to_string((long long)bytes) + " B";
    else
    {
        int exp = (int) (log(bytes) / log(unit));
        pre[0] = punits1[exp-1];
        pre[1] = punits2;
        pre[2] = '\0';
        float val = bytes / pow(unit, exp);
        sprintf(cBytes, "%.1f %sB", val, pre);
        hBytes = cBytes;
    }

    return *new string(hBytes);
}

const std::string& hBytes(u64 bytes)
{
    return humanReadableByteCount(bytes, true);
}