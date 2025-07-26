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