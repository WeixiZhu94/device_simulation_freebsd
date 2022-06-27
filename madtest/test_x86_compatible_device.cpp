#include "accelerator_runtime.h"

const char* test_name[] = {
    "vector_add",
    "crc",
};


int main(int argc, char **argv)
{
	int num_of_tests = 1;
	kernel_instance test_program[1] = {SUM};
	void *kernel_args;
	int ret;

	// This test program simulates a program run on a peripheral MMU that is compatible with an x86-64 MMU
	printf("[Test]: Creating context\n");
	clContextCreate(SHARE_CPU);

	for (int i = 0; i < num_of_tests; i ++) {
		ret = generate_test(test_program[i], &kernel_args);
		if (ret)
			printf("Test generation failed\n");
		else
			printf("Test generation succeeded\n");

		clLaunchKernel(test_program[i], kernel_args);

		ret = validate_test(test_program[i], kernel_args);
		if (ret == 0)
			printf("Test %d passed. Test name: %s\n", i, test_name[test_program[i]]);
		else
			printf("Test %d failed. Test name: %s\n", i, test_name[test_program[i]]);
	}

	return 0;
}