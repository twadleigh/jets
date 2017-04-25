#include <iostream>
#include <jlts.h>

int main() {
    jlts_init();

    double io[3] = {1.0, 2.0, 0.0};
    jlts_call(io);
    std::cout << io[2] << std::endl;
    jlts_finalize();
    return 0;
}
