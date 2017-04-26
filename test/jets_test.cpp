#include <iostream>
#include <jets.h>

const char * const g_expr = "include(\"C:/tw/jets/jl/boot.jl\")";

struct jets_manager {
    jets_manager() { jets_init(g_expr); }
    ~jets_manager() { jets_teardown(); }
} g_jets_manager;

int main() {
    double io = 0.0;
    jets_call(&io);
    std::cout << io << std::endl;
    return 0;
}
