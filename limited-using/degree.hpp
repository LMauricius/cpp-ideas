// degree.hpp

#include "timings.hpp"

// Let's assume that this header is included after timings.hpp...

class Degree
{
public:
    Degree(int);

    //...
};

Degree operator"" d(unsigned long long a)
{
    return Degree(a);
}