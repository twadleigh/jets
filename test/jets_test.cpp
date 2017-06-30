#include <iostream>
#include <jets.h>

struct jets_manager {
    jets_manager() { jets_init(); }
    ~jets_manager() { jets_teardown(); }
} g_jets_manager;

void my_cb(void *arg) {
    double *dbl = (double *)arg;
    *dbl = 21.0;
}

int main() {
    double val = 0.0;
    jets_exec(my_cb, &val);
    std::cout << val << std::endl;
}
