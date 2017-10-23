#include <iostream>
#include <jets.h>

void my_cb(void *arg) {
    double *dbl = (double *)arg;
    *dbl = 21.0;
}

int main() {
	if (jets_init() < 0) {
		std::cout << "Thread initialization failed!" << std::endl;
		exit(1);
	}

    double val = 0.0;
    jets_exec(my_cb, &val);
    std::cout << val << std::endl;

	jets_teardown();
}
