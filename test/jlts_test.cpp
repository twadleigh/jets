#include <iostream>
#include <jlts.h>

int main() {
    double io = 0.0;
    jlts_call(&io);
    std::cout << io << std::endl;
    return 0;
}
