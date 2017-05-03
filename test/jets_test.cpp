#include <iostream>
#include <jets.h>

struct jets_manager {
    jets_manager() { jets_init(); }
    ~jets_manager() { jets_teardown(); }
} g_jets_manager;

int main() {
    jl_value_t *val = jets_eval_string("sin(pi)");
    std::cout << jl_unbox_float64(val) << std::endl;
}
