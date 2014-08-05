#include "utility.h"

bool isEmpty(std::ifstream& fin)
{
    return fin.peek() == std::ifstream::traits_type::eof();
}
