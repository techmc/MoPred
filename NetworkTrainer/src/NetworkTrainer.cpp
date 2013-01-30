//============================================================================
// Name        : NetworkTrainer.cpp
// Author      : Zassa Kavuma
// Version     : 0.2
// Copyright   : Released under the GNU General Public License
// Description : Program for training neural networks
//============================================================================

// Includes
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <fann.h>

// Defines
#define TOTAL 240 // total number of training data sets 40*6
#define INPUT 40
#define OUTPUT 6
#define HIDDEN 23

// Globals
using namespace std;

int FANN_API test_callback(struct fann *ann, struct fann_train_data *train,
	unsigned int max_epochs, unsigned int epochs_between_reports,
	float desired_error, unsigned int epochs)
{
	printf("Epochs     %8d. MSE: %.5f. Desired-MSE: %.5f\n",
	       epochs, fann_get_MSE(ann), desired_error);
	return 0;
}

int main()
{
	const unsigned int num_input = 	INPUT; // Number of input neurons
	const unsigned int num_output = OUTPUT; // Number of output neurons
	const float desired_error = (const float) 0;
	const unsigned int epochs_between_reports = 100;
	struct fann *ann;
	struct fann_train_data *data1, *data2, *data3, *data4, *data5, *data6, *datam1, *datam2, *datam3, *data, *dataz;

	unsigned int num_neurons_hidden; // Number of neurons in the hidden layer
	unsigned int max_epochs;

	// Create neural network
	printf("Creating network.\n");

	// Import training data
	data1 = fann_read_train_from_file("../Shared/DCT/DCT000001.data");
	data2 = fann_read_train_from_file("../Shared/DCT/DCT000010.data");
	data3 = fann_read_train_from_file("../Shared/DCT/DCT000100.data");
	data4 = fann_read_train_from_file("../Shared/DCT/DCT001000.data");
	data5 = fann_read_train_from_file("../Shared/DCT/DCT010000.data");
	data6 = fann_read_train_from_file("../Shared/DCT/DCT100000.data");
	datam1 = fann_merge_train_data(data1, data2); // Merge data
	datam2 = fann_merge_train_data(data3, data4);
	datam3 = fann_merge_train_data(data5, data6);
	data = fann_merge_train_data(datam1, datam2); // Merge data
	data = fann_merge_train_data(data, datam3);
	fann_shuffle_train_data(data); // Shuffle data

	// Scale the training data
	float big, little;
	unsigned int i, j;

	for( j = 0 ; j < TOTAL ; j++ ) {
		big = -1000000;
		little = 1000000;
		for( i = 0 ; i < num_input ; i++ ) {
				if (big < data->input[j][i]) big = data->input[j][i];
				if (little > data->input[j][i]) little = data->input[j][i];
			}
		
		for( i = 0 ; i < num_input ; i++ ) {
			data->input[j][i]=(data->input[j][i]-little)/(big-little);
			}
	}

	stringstream ss;
	string track;
	int x = 0;
	float lim;
	ofstream myfile1;
	myfile1.open ("../Shared/BAG/Bagging.data");
	max_epochs = 171;
	
	for (lim = 1; lim <= 240; lim=lim+1)
	{
		num_neurons_hidden = 27;
		ann = fann_create_standard(4, num_input, num_neurons_hidden, num_neurons_hidden, num_output);
		fann_set_activation_steepness_hidden(ann, 1); // Set activation steepness
		fann_set_activation_steepness_output(ann, 1);
		fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC); // Set activation function
		fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
		fann_set_train_stop_function(ann, FANN_STOPFUNC_BIT); // Set the training stop function
		fann_set_bit_fail_limit(ann, 0.35f); //0.7 at best
		fann_randomize_weights(ann, -0.02, 0.0); // Randomise weights
		fann_init_weights(ann, data); // Set initial weights

		// Run training function
		printf("Training network.\n");
		cout << "1st layer neurons: " << lim << endl;
		dataz  = fann_subset_train_data(data, 0, lim);
		fann_train_on_data(ann, dataz, max_epochs, epochs_between_reports, desired_error);
		
		// Save neural network
		printf("Saving network.\n");
		ss << x;
		track = ss.str();
		string basename = "../Shared/MopredX.net";
		basename.replace(16, 1, track);
		cout<<basename<<endl;
		fann_save(ann, basename.c_str());
		ss.str("");

		myfile1 << lim << endl;
		x++;
		}
	printf("Cleaning up.\n");
	myfile1.close();
	fann_destroy_train(data);
	fann_destroy(ann);
	return 0;
}
