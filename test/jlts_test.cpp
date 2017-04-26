#include <iostream>
#include <jlts.h>

int main() {
    jlts_init("include(\"C:/tw/jlts/jl/boot.jl\")");
    double io = 0.0;
    jlts_call(&io);
    std::cout << io << std::endl;
    jlts_teardown();
    return 0;
}
