#include <iostream>
#include <jets.h>

int main() {
    jets_init("include(\"C:/tw/jets/jl/boot.jl\")");
    double io = 0.0;
    jets_call(&io);
    std::cout << io << std::endl;
    jets_teardown();
    return 0;
}
