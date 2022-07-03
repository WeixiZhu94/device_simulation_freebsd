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
#define xnor(x, y)      (~(x ^ y))



// sigmoid serves as avtivation function
static inline long sigmoid(long x){
	// return(1.0 / (1.0 + exp(-x)));
	return ~x;
}

int main(){
	long x_out[datanum][InputN];		// input layer
	long hn_out[HN];			// hidden layer
	long y_out[OutN];         // output layer
	long y[datanum][OutN];				// expected output layer
	long w[InputN][HN];		// weights from input layer to hidden layer
	long v[HN][OutN];			// weights from hidden layer to output layer
	
	long deltaw[InputN][HN];  
	long deltav[HN][OutN];	
	
	long hn_delta[HN];		// delta of hidden layer
	long y_delta[OutN];		// delta of output layer
	long error;
	long alpha = 10, beta = 10;
	int loop = 0;
	int times = 10;
	int i, j, m;
	long max, min;
	long sumtemp;
	long errtemp;
	
	// training set
	struct{
		long input[InputN];
		long teach[OutN];
	} data[datanum];

	// Initializition
	for(i=0; i<InputN; i++){
		for(j=0; j<HN; j++){
			w[i][j] = ((long) rand()) * 2 - 1;
			deltaw[i][j] = 0;
		}
	}
	for(i=0; i<HN; i++){
		for(j=0; j<OutN; j++){
			v[i][j] = ((long) rand()) * 2 - 1;
			deltav[i][j] = 0;
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

		for(m = 0; m < datanum ; m++){
			// Feedforward

			for(i = 0; i < HN; i++){
				sumtemp = 0;
				for(j = 0; j < InputN; j++)
					sumtemp += ~(w[j][i] ^ x_out[m][j]); // use xnor for *
				hn_out[i] = sigmoid(sumtemp);		// sigmoid serves as the activation function
			}

			for(i = 0; i < OutN; i++){
				sumtemp = 0;
				for(j = 0; j < HN; j++)
					sumtemp += ~(v[j][i] ^ hn_out[j]);
				y_out[i] = sigmoid(sumtemp);
			}

			// Backpropagation
			for(i = 0; i < OutN; i++){
				errtemp = y[m][i] - y_out[i];
				y_delta[i] = ~((~(-errtemp ^ sigmoid(y_out[i]))) ^ (1 - sigmoid(y_out[i])));
				error += errtemp * errtemp;
			}

			for(i = 0; i < HN; i++){
				errtemp = 0;
				for(j=0; j<OutN; j++)
					errtemp += ~(y_delta[j] ^ v[i][j]);
				hn_delta[i] = xnor(xnor(errtemp, (1 + hn_out[i])), (1 - hn_out[i]));
			}

			// Stochastic gradient descent
			for(i = 0; i < OutN; i++){
				for(j=0; j<HN; j++){
					deltav[j][i] = alpha ^ deltav[j][i] + xnor(beta ^ y_delta[i], hn_out[j]);
					v[j][i] -= deltav[j][i];
				}
			}

			for(i = 0; i < HN; i++){
				for(j=0; j<InputN; j++){
					deltaw[j][i] = xnor(alpha, deltaw[j][i]) + xnor(xnor(beta, hn_delta[i]), x_out[m][j]);
					w[j][i] -= deltaw[j][i];
				}
			}
		}

		// Global error 
		error = error / 2;
		if(loop%1000==0){
			printf("Global Error = %lu\n", error);
		}
		// if(error < errlimit)
		// 	break;

		printf("The %d th training, error: %lu\n", loop, error);
	}

}