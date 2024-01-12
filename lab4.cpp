#include <iostream>

#include "lab4.h"


int main(int argc, char *argv[])
{
    if (argc > 1)
        std::cout << "Command-line argument: " << argv[1] << std::endl;
    return 0;
}
