#include "accelerator_runtime.h"

#define TEST_LENGTH 100*1024*1024

int generate_test(kernel_instance kernel, void **kernel_args)
{
	if (kernel_args == NULL) {
		printf("No output kernel args\n");
		return -1;
	}

	if (kernel == SUM) {
		struct vector_add_args *args;
		args = (struct vector_add_args*)malloc(sizeof(struct vector_add_args));
		args->a = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->b = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->c = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->len = TEST_LENGTH;

		*kernel_args = (void*) args;
		if (args->a && args->b && args->c)
			return 0;
		else
			return -ENOMEM;

	} else if (kernel == CRC) {
		printf("CRC test not implemented\n");
		return -1;
	}
	printf("Unknown kernel type\n");
	return -1;
}


int validate_test(kernel_instance kernel, void *kernel_args)
{
	if (kernel == SUM) {
		struct vector_add_args *args = (struct vector_add_args*) kernel_args;
		uint64_t *a = args->a, *b = args->b, *c = args->c;
		int i;
		for (i = 0; i < TEST_LENGTH; i ++)
			if (!(c[i] == a[i] + b[i]))
				break;
		if (i == TEST_LENGTH)
			return 0;
		else
			return i;

	} else if (kernel == CRC) {
		printf("CRC test not implemented\n");
		return -1;
	}
	printf("Unknown kernel type\n");
	return -1;
}