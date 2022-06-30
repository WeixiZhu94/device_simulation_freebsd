#include "accelerator_runtime.h"

#define TEST_LENGTH 100*1024*1024

int generate_test(kernel_instance kernel, void **kernel_args)
{
	if (kernel_args == NULL) {
		printf("No output kernel args\n");
		return -1;
	}

	if (kernel == SUM) {
		struct accelerator_kernel_args *args;
		args = (struct accelerator_kernel_args*) malloc(sizeof(struct accelerator_kernel_args));
		args->kernel_type = kernel;
		args->vector_add.a = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->vector_add.b = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->vector_add.c = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->vector_add.len = TEST_LENGTH;

		printf("%s %d, args: %p, a %p, b %p, c %p, len %lu\n", __func__, __LINE__, 
			args, 
			args->vector_add.a, 
			args->vector_add.b, 
			args->vector_add.c, 
			args->vector_add.len);
		// *kernel_args = (void*) args;
		*kernel_args = (void *) args;
		if (args->vector_add.a != NULL && args->vector_add.b != NULL && args->vector_add.c != NULL) {
			for (uint64_t i = 0; i < args->vector_add.len; i ++) {
				args->vector_add.a[i] = (i * 13 + 9689) % 10007;
				args->vector_add.b[i] = (i * 7 + 9689) % 10007;
			}
			return 0;
		}
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
	struct accelerator_kernel_args *args;
	args = (struct accelerator_kernel_args*) kernel_args;
	if (kernel == SUM) {
		int i;
		for (i = 0; i < args->vector_add.len; i ++)
			if (!(args->vector_add.c[i] == args->vector_add.a[i] + args->vector_add.b[i]))
				break;
		if (i == args->vector_add.len)
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