#ifndef PRINT_H
#define PRINT_H

#include <iostream>

// generic print function
template <typename T>
void print(T t) { std::cout << t << std::endl; }

// print function for multiple arguments
template <typename T, typename... Args>
void print(T t, Args... args)
{
    std::cout << t << " ";
    print(args...);
}

#endif // PRINT_H
