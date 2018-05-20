#include "pit/App.hpp"
#include <iostream>

int main(int argc, const char **argv)
{
    pit::App app;

    if (!app.process(argc, argv))
    {
        std::cout << "Error: could not process the app" << std::endl;
        return -1;
    }

    std::cout << "Everything went OK" << std::endl;
    return 0;
}
