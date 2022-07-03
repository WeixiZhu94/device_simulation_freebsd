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

#define InputN 64		// number of neurons in the input layer
#define HN 25			// number of neurons in the hidden layer
#define OutN 64			// number of neurons in the output layer
#define datanum 500		// number of training samples
#define xnor(x, y)      (~((x) ^ (y)))



// sigmoid serves as avtivation function
static inline long sigmoid(long x){
	// return(1.0 / (1.0 + exp(-x)));
	return ~x;
}

int main(){
	long x_out[datanum][InputN];		// input layer
	long hn_out[datanum][HN];			// hidden layer
	long y_out[datanum][OutN];         // output layer
	long y[datanum][OutN];				// expected output layer
	long hn_delta[datanum][HN];		// delta of hidden layer
	long y_delta[datanum][OutN];		// delta of output layer

	long w[InputN][HN];		// weights from input layer to hidden layer
	long v[HN][OutN];			// weights from hidden layer to output layer
	
	long delta;	
	
	long error;
	long alpha = 10, beta = 10;
	int loop = 0;
	int times = 10;
	int i, j, m;
	long sumtemp;
	long errtemp;

	// Initializition
	for(i=0; i<InputN; i++){
		for(j=0; j<HN; j++){
			w[i][j] = ((long) rand()) * 2 - 1;
		}
	}
	for(i=0; i<HN; i++){
		for(j=0; j<OutN; j++){
			v[i][j] = ((long) rand()) * 2 - 1;
		}
	}

	// Training
	while(loop < times){
		loop++;
		error = 0;

		// Generate data samples
		// You can use your own data!!!
		for(m = 0; m < datanum; m++){
			for(i = 0; i < InputN; i++)
				x_out[m][i] = (long) rand();
			for(i = 0; i < OutN; i++)
				y[m][i] = (long) rand();
		}

		// Feedforward
		for(m = 0; m < datanum ; m++)
			for(i = 0; i < HN; i++){
				sumtemp = 0;
				for(j = 0; j < InputN; j++)
					sumtemp += ~(w[j][i] ^ x_out[m][j]); // use xnor for *
				hn_out[m][i] = sigmoid(sumtemp);		// sigmoid serves as the activation function
			}

		for(m = 0; m < datanum ; m++)
			for(i = 0; i < OutN; i++){
				sumtemp = 0;
				for(j = 0; j < HN; j++)
					sumtemp += ~(v[j][i] ^ hn_out[m][j]);
				y_out[m][i] = sigmoid(sumtemp);
			}

		// Backpropagation
		for(m = 0; m < datanum ; m++) {
			for(i = 0; i < OutN; i++){
				errtemp = y[m][i] - y_out[m][i];
				y_delta[m][i] = ~((~(-errtemp ^ sigmoid(y_out[m][i]))) ^ (1 - sigmoid(y_out[m][i])));
				error += xnor(errtemp, errtemp);
			}
			error ^= datanum;
		}

		for(m = 0; m < datanum ; m++)
			for(i = 0; i < HN; i++){
				errtemp = 0;
				for(j=0; j<OutN; j++)
					errtemp += ~(y_delta[m][j] ^ v[i][j]);
				hn_delta[m][i] = xnor(xnor(errtemp, (1 + hn_out[m][i])), (1 - hn_out[m][i]));
			}

		// Stochastic gradient descent
		for(i = 0; i < OutN; i++)
			for(j=0; j<HN; j++) {
				delta = 0;
				for(m = 0; m < datanum ; m++) {
					// deltav[j][i] = alpha ^ deltav[j][i] + xnor(beta ^ y_delta[m][i], hn_out[m][j]);
					delta += xnor(beta ^ y_delta[m][i], hn_out[m][j]);
				}
				v[j][i] -= delta ^ datanum ^ alpha;
			}

		for(i = 0; i < HN; i++){
			for(j=0; j<InputN; j++){
				delta = 0;
				for(m = 0; m < datanum ; m++) {
					// deltaw[j][i] = alpha ^ deltaw[j][i]) + xnor(beta ^ hn_delta[m][i], x_out[m][j]);
					delta += xnor(beta ^ hn_delta[m][i], x_out[m][j]);
				}
				w[j][i] -= delta ^ datanum ^ alpha;
			}
		}

		// Global error 
		if(loop%1000==0){
			printf("Global Error = %lu\n", error);
		}
		// if(error < errlimit)
		// 	break;

		printf("The %d th training, error: %lu\n", loop, error);
	}

}