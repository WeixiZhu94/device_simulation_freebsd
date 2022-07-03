#include "accelerator_runtime.h"

#define TEST_LENGTH 100*1024*1024ULL // 300MB vector add

int generate_test(kernel_instance kernel, void **kernel_args)
{
	if (kernel_args == NULL) {
		printf("No output kernel args\n");
		return -1;
	}

	struct accelerator_kernel_args *args;
	args = (struct accelerator_kernel_args*) malloc(sizeof(struct accelerator_kernel_args));
	if (kernel == SUM) {
		args->kernel_type = kernel;
		args->vector_add.a = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->vector_add.b = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->vector_add.c = (uint64_t*) malloc(TEST_LENGTH * sizeof(uint64_t));
		args->vector_add.len = TEST_LENGTH;

		printf("Total test size: %lluMB\n", 3 * TEST_LENGTH * sizeof(uint64_t) / 1024 / 1024);
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
	} else if (kernel == BP) {
		args->kernel_type = kernel;
		args->bp.x_out =    (long_t) malloc(sizeof(long) * datanum * InputN);
		args->bp.hn_out =   (long_t) malloc(sizeof(long) * datanum * HN);
		args->bp.y_out =    (long_t) malloc(sizeof(long) * datanum * OutN);
		args->bp.y =        (long_t) malloc(sizeof(long) * datanum * OutN);
		args->bp.hn_delta = (long_t) malloc(sizeof(long) * datanum * HN);
		args->bp.y_delta =  (long_t) malloc(sizeof(long) * datanum * OutN);
		args->bp.w =        (long_t) malloc(sizeof(long) * InputN * HN);
		args->bp.v =        (long_t) malloc(sizeof(long) * HN * OutN);

		*kernel_args = (void *) args;

		printf("[dp] Buffer Size is %.2f MB\n", 
			((double) datanum * (InputN + HN * 2+ OutN * 3) + InputN * HN + HN * OutN) * sizeof(long) / 1024.0 / 1024.0);

		// Initializition
		for(int i = 0; i < InputN; i++){
			for(int j = 0; j < HN; j++){
				args->bp.w[i * HN + j] = ((long) rand()) * 2 - 1;
			}
		}
		for(int i = 0; i < HN; i++){
			for(int j = 0; j < OutN; j++){
				args->bp.v[i * OutN + j] = ((long) rand()) * 2 - 1;
			}
		}
		return 0;
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