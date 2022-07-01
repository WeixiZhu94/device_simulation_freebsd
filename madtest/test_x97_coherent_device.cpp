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
	clContextCreate(REPLICATE_CPU);

	for (int i = 0; i < num_of_tests; i ++) {
		printf("Test: %s\n", test_name[i]);
		ret = generate_test(test_program[i], &kernel_args);
		printf("Done with test generation\n");
		if (ret)
			printf("Test generation failed\n");
		else
			printf("Test generation succeeded\n");

		printf("Launching kernel\n");
		clLaunchKernel(test_program[i], kernel_args);

		ret = validate_test(test_program[i], kernel_args);
		if (ret == 0)
			printf("Test %s passed. \n", test_name[test_program[i]]);
		else
			printf("Test %s failed. \n", test_name[test_program[i]]);
	}

	return 0;
}