# limited-using declaration

## Introduction
In C++ it is widely considered a bad practice to use the `using` keyword to shorten the code and for a good reason: it defeats the purpose of namespaces.
While it is often used to alias types as a clearer version of C's `typedef`{.c} keyword,
using it to just make the implementation clearer to follow also affects the program's API.
Even worse, `using namespace`{.cpp} can have unwanted and unpredictable consequences
as it includes any and all declarations from the specified namespaces of which the user might not be aware, and that could be extended by the namespace's authors at any point in future. The only use-case which is somewhat safe is redirecting one namespace controlled by the author to another, which is rarely seen in C++ codebases.

As such this seemingly useful construct required to be implemented by compiler authors and learned by C++ programmers is almost useless in practice. This paper aims to solve all the dangers of this keyword while allowing much more readable code without compromising on explicitness of usage.

## Examples of current standard

### The simplest example
Let's start with a *counter*-example that is easy enough to solve, the "Hello world!" example:
```cpp
#include <print>

using namespace std;

int main() {
    print("Hello, {}!\n", "world");
}
```

If usage of `using namespace`{.cpp} is forbidden, we get a *slightly* more verbose function definition: 
```cpp
#include <print>

int main() {
    std::print("Hello, {}!\n", "world");
}
```

Curiously enough, this code snippet is still shorter by two lines. Obviously the new features such as C++23's `std::print`{.cpp} simplify the code enough that namespace qualifiers are no longer a big factor. But as we will see, there are many examples where introducing convenience functions doesn't cut it.

### A conflicting use-case
One use-case where the existing limitations and dangers of the `using`{.cpp} keyword complicate the matters is the `std::chrono`{.cpp} library:
```cpp
// timings.hpp

#include <chrono>

using namespace std::chrono_literals;

constexpr std::chrono::steady_clock::duration Period = 500ms;
constexpr std::chrono::steady_clock::duration TotalDuration = 1h;
```

As this header includes the whole `std::chrono_literals`{.cpp} namespace,
it is dangerous to use in practice. An unaware user might try to define a conflicting literal operator:
```cpp
// degree.hpp

// Let's assume that this header is included after timings.hpp...

class Degree
{
public:
    Degree(int);

    //...
};

// Error: cannot overload functions distinguished by return type alone
Degree operator"" d(unsigned long long a)
{
    return Degree(a);
}
```

An important note is that the standard specifies that the user *should not* declare new literal operators without a leading underscore. While replacing `d` with a `_d` solves the conflicts with the standard library, it doesn't fix the possible conflicts with other libraries which might have other operators with the same name.
For simplicity this example will still compare

The only completely safe solution would be to not include the namespace at all.
```cpp
// timings.hpp

#include <chrono>

constexpr std::chrono::steady_clock::duration Period = std::chrono_literals::operator""ms(500);
constexpr std::chrono::steady_clock::duration TotalDuration = std::chrono_literals::operator""h(1);
```

Obviously, this defeats the very purpose of user-defined literals: to make the code more compact, readable and obvious.

### A more extreme reduction
A more convoluted example:
```cpp
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

int main()
{
    vector<string> words = {"C++", "is", "insane"};

    cout << transform_reduce(
                next(words.begin()), words.end(), words.front(),
                [](string a, const string &b)
                { return move(a) + "-" + b; },
                [](const string &s)
                { return s; }
            )
         << " | total chars: "
         << transform_reduce(words.begin(), words.end(), 0, plus<>(),
                [](const string &s)
                { return (int)s.size(); }
            )
         << endl;
}
```

Obviously enough, the code prints the `words`' elements separated by dashes, before printing the total character count. 
Without the `using namespace`, this code becomes:
```cpp
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

int main()
{
    std::vector<std::string> words = {"C++", "is", "insane"};

    std::cout << std::transform_reduce(
                    next(words.begin()), words.end(), words.front(),
                    [](std::string a, const std::string &b)
                    { return std::move(a) + "-" + b; },
                    [](const std::string &s)
                    { return s; }
                 )
              << " | total chars: "
              << std::transform_reduce(
                    words.begin(), words.end(), 0, std::plus<>(),
                    [](const std::string &s)
                    { return (int)s.size(); }
                 )
              << std::endl;
}
```

With a possible shift towards more functional-style and data-oriented programming the current best-practices of C++ are a hindrance to embracing the newer and safer paradigms.

## Introducing the limited-using declarations and directives
To solve those conflicts I propose an extension of existing `using`{.cpp} constructs:
limiting their effect on explicit code.

### Syntax
A limited-using declaration is written as follows:
```cpp
using declarator-list { affected-code }
```

A limited-using enum declaration is written as follows:
```cpp
using enum using-enum-declarator { affected-code }
```

A limited-using directive is written as follows:
```cpp
using namespace nested-name-specifier /*optional*/ namespace-name { affected-code }
```

### Behavior
The limited-using declarations behave as if their non-limited equivalents are placed before the affected-code.
Unlike the non-limited equivalents, the names introduced by the `using`{.cpp} declarations are not visible
outside the limited-using declarations.

### NOT a scope
The limited-using declarations do not start a namespace or block scope.
For example, this is valid:
```cpp
using namespace std {
    void foo(const string &s) {
        print(s);
    }
}

void bar(const string &s) {
    foo(s); // foo is visible here
}
```

### Alternative specification
We could also specify the behavior as follows:
```cpp
using namespace std {
    int foo();
    int bar();
}
```
...is equivalent to:
```cpp
namespace __affected_code__ {
    using namespace std;
    int foo();
    int bar();
}
using __affected_code__::foo;
using __affected_code__::bar;
```

## Examples

### Hello world example
The simplest example:
```cpp
#include <print>

using std::print {
    int main() {
        print("Hello, {}!\n", "world");
    }
}
```

### std::chrono_literals example
We can now use user-defined literals without polluting the rest of the scope:
```cpp
// timings.hpp

#include <chrono>

using namespace std::chrono_literals {
    constexpr std::chrono::steady_clock::duration Period = 500ms;
    constexpr std::chrono::steady_clock::duration TotalDuration = 1h;
}
```

```cpp
// degree.hpp

// Let's assume that this header is included after timings.hpp...

class Degree
{
public:
    Degree(int);

    //...
};

// OK: std::chrono_literals::operator""d(unsigned long long) is not visible from here
Degree operator"" d(unsigned long long a)
{
    return Degree(a);
}
```

### A convoluted example
The third example could be made both explicit and clear:
```cpp
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

using std::vector,
      std::string,
      std::transform_reduce,
      std::next,
      std::move,
      std::plus {
    int main()
    {
        vector<string> words = {"C++", "is", "insane"};

        cout << transform_reduce(
                    next(words.begin()), words.end(), words.front(),
                    [](string a, const string &b)
                    { return move(a) + "-" + b; },
                    [](const string &s)
                    { return s; }
                )
            << " | total chars: "
            << transform_reduce(words.begin(), words.end(), 0, plus<>(),
                    [](const string &s)
                    { return (int)s.size(); }
                )
            << endl;
    }
}
```