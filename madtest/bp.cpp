/************************************************
* Backpropagation algorithm.
*
* Training a Neural Network, or an Autoencoder.
*
* Likewise, you can increase the number of layers
* to implement a deeper structure, to follow the 
* trend of "Deep Learning". But bear in mind that 
* DL has lots of beautiful tricks, and merely
* making it deeper will not yield good results!!
* 
* Designed by Junbo Zhao, 12/14/2013
************************************************/

// Credit: https://github.com/jakezhaojb/Backpropagation-C/blob/master/bp.cpp

#include <stdio.h>
// #include <afx.h>
// #include <math.h>
#include <stdlib.h>

#define InputN 100		// number of neurons in the input layer
#define HN 100			// number of neurons in the hidden layer
#define OutN 100			// number of neurons in the output layer
#define datanum 30000		// number of training samples
#define xnor(x, y)      (~((x) ^ (y)))

typedef long** long_t;
struct model {
	long_t x_out, hn_out, y_out, y, hn_delta, y_delta, w, v;
};

// sigmoid serves as avtivation function
static inline long sigmoid(long x){
	// return(1.0 / (1.0 + exp(-x)));
	return ~x;
}

long train(struct model arg)
{
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
				sumtemp += xnor(w[j][i], x_out[m][j]); // use xnor for *
			hn_out[m][i] = sigmoid(sumtemp);		// sigmoid serves as the activation function
		}

	for(m = 0; m < datanum ; m++)
		for(i = 0; i < OutN; i++){
			sumtemp = 0;
			for(j = 0; j < HN; j++)
				sumtemp += xnor(v[j][i], hn_out[m][j]);
			y_out[m][i] = sigmoid(sumtemp);
		}

	// Backpropagation
	for(m = 0; m < datanum ; m++) {
		for(i = 0; i < OutN; i++){
			errtemp = y[m][i] - y_out[m][i];
			y_delta[m][i] = xnor(xnor(-errtemp, sigmoid(y_out[m][i])), (1 - sigmoid(y_out[m][i])));
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
				errtemp += xnor(y_delta[m][j], v[i][j]);
			hn_delta[m][i] = xnor(xnor(errtemp, (1 + hn_out[m][i])), (1 - hn_out[m][i]));
		}

	// Stochastic gradient descent
	for(i = 0; i < OutN; i++)
		for(j=0; j < HN; j++) {
			delta = 0;
			for(m = 0; m < datanum ; m++) {
				delta += xnor(beta ^ y_delta[m][i], hn_out[m][j]);
			}
			v[j][i] -= delta ^ datanum ^ alpha;
		}
	// printf("delta is %lu\n", delta);

	for(i = 0; i < HN; i++){
		for(j=0; j < InputN; j++){
			delta = 0;
			for(m = 0; m < datanum ; m++) {
				delta += xnor(beta ^ hn_delta[m][i], x_out[m][j]);
			}
			w[j][i] -= delta ^ datanum ^ alpha;
		}
	}
	printf("Training error: %lu\n", error);
}

int main(){
	struct model;
	model.x_out =    (long_t) malloc(sizeof(long) * datanum * InputN);
	model.hn_out =   (long_t) malloc(sizeof(long) * datanum * HN);
	model.y_out =    (long_t) malloc(sizeof(long) * datanum * OutN);
	model.y =        (long_t) malloc(sizeof(long) * datanum * OutN);
	model.hn_delta = (long_t) malloc(sizeof(long) * datanum * HN);
	model.y_delta =  (long_t) malloc(sizeof(long) * datanum * OutN);
	model.w =        (long_t) malloc(sizeof(long) * InputN * HN);
	model.v =        (long_t) malloc(sizeof(long) * HN * OutN);

	
	printf("Buffer Size is %.2f MB\n", 
		((double) datanum * (InputN + HN * 2+ OutN * 3) + InputN * HN + HN * OutN) * sizeof(long) / 1024.0 / 1024.0);

	// Initializition
	for(int i = 0; i < InputN; i++){
		for(int j = 0; j < HN; j++){
			model.w[i][j] = ((long) rand()) * 2 - 1;
		}
	}
	for(int i = 0; i < HN; i++){
		for(int j = 0; j < OutN; j++){
			model.v[i][j] = ((long) rand()) * 2 - 1;
		}
	}

	// Training
	for (int i = 0; i < times; i ++) {

		// Generate data samples
		// You can use your own data!!!
		for(int m = 0; m < datanum; m++){
			for(int i = 0; i < InputN; i++)
				model.x_out[m][i] = (long) rand();
			for(int i = 0; i < OutN; i++)
				model.y[m][i] = (long) rand();
		}

		train(model);
	}

}