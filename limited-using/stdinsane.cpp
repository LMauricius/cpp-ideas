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