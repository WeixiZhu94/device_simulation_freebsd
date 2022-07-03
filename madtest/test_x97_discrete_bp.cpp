#include "accelerator_runtime.h"
#include "maddefs.h"

const char* test_name[] = {
    "vector_add",
    "crc",
    "bp",
};


int num_of_tests = 1;
kernel_instance test_program[1] = {BP};

void generate_bp_input(struct accelerator_kernel_args *kernel_args)
{
	struct model arg = kernel_args->bp;

	for(int m = 0; m < datanum; m++){
		for(int i = 0; i < InputN; i++)
			arg.x_out[m * InputN + i] = (long) rand();
		for(int i = 0; i < OutN; i++)
			arg.y[m * OutN + i] = (long) rand();
	}
}

void cpu_bp(struct accelerator_kernel_args *kernel_args)
{
	struct model arg = kernel_args->bp;
	long_t x_out = arg.x_out;
	long_t hn_out = arg.hn_out;
	long_t y_out = arg.y_out;
	long_t y = arg.y;
	long_t hn_delta = arg.hn_delta;
	long_t y_delta = arg.y_delta;
	long_t w = arg.w;
	long_t v = arg.v;

	long error = 0;	
	long alpha = 10; 
	long beta = 10;
	long delta, sumtemp, errtemp;	
	int i, j, m;

	// Feedforward
	for(m = 0; m < datanum ; m++)
		for(i = 0; i < HN; i++){
			sumtemp = 0;
			for(j = 0; j < InputN; j++)
				sumtemp += xnor(w[j * HN + i], x_out[m * InputN + j]); // use xnor for *
			hn_out[m * HN + i] = sigmoid(sumtemp);		// sigmoid serves as the activation function
		}

	for(m = 0; m < datanum ; m++)
		for(i = 0; i < OutN; i++){
			sumtemp = 0;
			for(j = 0; j < HN; j++)
				sumtemp += xnor(v[j * OutN + i], hn_out[m * HN + j]);
			y_out[m * OutN + i] = sigmoid(sumtemp);
		}

	// Backpropagation
	for(m = 0; m < datanum ; m++) {
		for(i = 0; i < OutN; i++){
			errtemp = y[m * OutN + i] - y_out[m * OutN + i];
			y_delta[m * OutN + i] = xnor(xnor(-errtemp, sigmoid(y_out[m * OutN + i])), (1 - sigmoid(y_out[m * OutN + i])));
			// error += xnor(errtemp, errtemp);
			error += errtemp * errtemp;
		}
		error /= OutN;
	}
	error /= datanum;

	for(m = 0; m < datanum ; m++)
		for(i = 0; i < HN; i++){
			errtemp = 0;
			for(j=0; j<OutN; j++)
				errtemp += xnor(y_delta[m * OutN +j], v[i * OutN +j]);
			hn_delta[m * HN + i] = xnor(xnor(errtemp, (1 + hn_out[m * HN + i])), (1 - hn_out[m * HN + i]));
		}

	// Stochastic gradient descent
	for(i = 0; i < OutN; i++)
		for(j = 0; j < HN; j++) {
			delta = 0;
			for(m = 0; m < datanum ; m++) {
				delta += xnor(beta ^ y_delta[m * OutN + i], hn_out[m * HN + j]);
			}
			v[j * OutN + i] -= delta ^ datanum ^ alpha;
		}
	// printf("delta is %lu\n", delta);

	for(i = 0; i < HN; i++){
		for(j = 0; j < InputN; j++){
			delta = 0;
			for(m = 0; m < datanum ; m++) {
				delta += xnor(beta ^ hn_delta[m * HN + i], x_out[m * InputN + j]);
			}
			w[j * HN + i] -= delta ^ datanum ^ alpha;
		}
	}
	printf("Training error: %lu\n", error);
	return error;
}

static void run_bp()
{
	int ret;
	void *kernel_args;
	ret = generate_test(test_program[i], &kernel_args);
	printf("Done with test initialization\n");
	if (ret)
		printf("Test generation failed\n");
	else
		printf("Test generation succeeded\n");

	for (int i = 0; i < 3; i ++) {
		printf("Training step %d\n", i);

		generate_bp_input((struct accelerator_kernel_args *) kernel_args);

		cpu_bp((struct accelerator_kernel_args *) kernel_args);
		// printf("Launching kernel\n");
		// clLaunchKernel(test_program[i], kernel_args);
	}
}

int main(int argc, char **argv)
{

	// This test program simulates a program run on a peripheral MMU that is compatible with an x86-64 MMU
	printf("[Test]: Creating context\n");
	clContextCreate(EXCLUSIVE);

	for (int i = 0; i < num_of_tests; i ++) {
		printf("Test: %s\n", test_name[i]);

		run_bp();
	}

	return 0;
}