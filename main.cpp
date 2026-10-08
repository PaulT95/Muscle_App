#include "Calgary_TAR/App.hpp"
#include <iostream>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    try {
        TARapp::App app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}