#include "utility.h"

bool isEmpty(std::ifstream& fin)
{
    return fin.peek() == std::ifstream::traits_type::eof();
}

const string& Target2RtnName(ADDRINT target)
{
    const string& name = RTN_FindNameByAddress(target);
    
    if (name == "")
        return *new string("[Unknown routine]");
    else
        return *new string(name);
}

const string& Target2LibName(ADDRINT target)
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
