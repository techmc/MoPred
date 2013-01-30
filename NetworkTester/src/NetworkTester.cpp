//============================================================================
// Name        : NetworkUser.cpp
// Author      : Zassa Kavuma
// Version     : 0.2
// Copyright   : Released under the GNU General Public License
// Description : Program for evaluating neural network performance
//============================================================================

// Includes
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <fann.h>

// Defines
#define TOTAL 120 // 20*6 
#define INPUT 40
#define PERMS 1600

// Globals
using namespace std;

int main()
{
	fann_type 		*calc_out;
	int 			ret = 0;
	unsigned int 	BFEtrack, BFEaccum, BFEleast, best;
	
	struct fann *ann;
	struct fann_train_data *data1, *data2, *data3, *data4, *data5, *data6, *datam1, *datam2, *datam3, *data;


	printf("Creating network.\n");

	// Import training data
	data1 = fann_read_train_from_file("../Shared/DCT/DCTV000001.data");
	data2 = fann_read_train_from_file("../Shared/DCT/DCTV000010.data");
	data3 = fann_read_train_from_file("../Shared/DCT/DCTV000100.data");
	data4 = fann_read_train_from_file("../Shared/DCT/DCTV001000.data");
	data5 = fann_read_train_from_file("../Shared/DCT/DCTV010000.data");
	data6 = fann_read_train_from_file("../Shared/DCT/DCTV100000.data");
	datam1 = fann_merge_train_data(data1, data2); // Merge data
	datam2 = fann_merge_train_data(data3, data4);
	datam3 = fann_merge_train_data(data5, data6);
	data = fann_merge_train_data(datam1, datam2); // Merge data
	data = fann_merge_train_data(data, datam3);
	fann_shuffle_train_data(data); // Shuffle data

	// Scale test data
	float big, little;
	unsigned int i, j;
	const unsigned int num_input = 	INPUT;

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
	BFEleast = 1000;

	fstream inFile;
	float parabuff[PERMS][2];
	inFile.open ("../Shared/BAG/Bagging.data"); //edit
	
	for (i = 0; i < PERMS; ++i) // for each row
	{
		inFile >> parabuff[i][0]; // Read file to row j column k
	}
	inFile.close(); //close file

	for (x=0; x<=7000; x++) // for each ANN
	{
		ss << x;
		track = ss.str();
		string basename = "../Shared/MopredX.net";
		basename.replace(16, 1, track);
		cout<<basename<<endl;
		ann = fann_create_from_file(basename.c_str());
		ss.str("");

		if(!ann)
		{
			printf("Error creating ann --- ABORTING.\n");
			cout << "Lowest BFEaccum: " << BFEleast << "  Best Mopred: " << best << endl;

			inFile.open ("../Shared/BAG/Bagging.data"); //edit
			for (i = 0; i < PERMS; ++i) // for each row
			{
				inFile << parabuff[i][0] << "\t" << parabuff[i][1] << endl;
			}
			inFile.close(); //close file

			return 0;
		}

		BFEtrack=0;
		BFEaccum=0;

		unsigned int result[6][7] = {0};
		unsigned int active_cnt;

		// Test network
		printf("Testing network.\n");
		for(i = 0; i < fann_length_train_data(data); i++) // for each pair in data pattern
		{
			fann_reset_MSE(ann);
			calc_out = fann_test(ann, data->input[i], data->output[i]); // Also outputs MSE/BFE

			// Save performance to array
			unsigned int y = 0;
			unsigned int mtrack = 0;
			active_cnt=0;

			for(j = 0; j < 6; j++) // search for output code
			{
				if (data->output[i][j]==1)
				{
					y = j;
				}
			}

			for (j = 0; j < 6; j++)
			{
				if (calc_out[j]>0)
				{
					active_cnt=active_cnt+1;
				}
			}
			if (active_cnt==0) // if 0-of-many
			{
				mtrack = result[y][6];
				result[y][6] = mtrack+1;
			}
			if (active_cnt>1) // if > 1-of-many
			{
				mtrack = result[y][6];
				result[y][6] = mtrack+1;
			}
			if (active_cnt==1) // if 1-of-many
			{
				for(j = 0; j < 6; j++) // search for output code
				{
					if (calc_out[j]>0)
					{
						mtrack = result[y][j];
						result[y][j] = mtrack+1;
					}
				}
			}
			// Check for errors
			for(j = 0; j < 6; j++) // set output counter
			{
				if (data->output[i][j]==1)
				{
					if (calc_out[j]<=0)
					{
						BFEtrack++;
					}
				}
				if (data->output[i][j]==-1)
				{
					if (calc_out[j]>=0)
					{
						BFEtrack++;
					}
				}
			}
			if (BFEtrack>0) // if single pair wrong
			{
				BFEaccum++;
			}
			BFEtrack=0;
		}

		cout << "BFEaccum: " << BFEaccum << endl; // display total wrong pairs in pattern
		parabuff[x][1]=BFEaccum; // save to buffer

		cout << endl;
		for(j = 0; j < 6; j++) // search for output code
		{
			cout << result[j][0] << "\t" << result[j][1] << "\t" << result[j][2] << "\t" << result[j][3] << "\t" << result[j][4] << "\t" << result[j][5] << "\t" << result[j][6] << endl;
		}
		cout << endl;

		if (BFEaccum<BFEleast)
		{
			BFEleast=BFEaccum;
			best=x;
			cout << "New lowest BFEaccum: " << BFEleast << "  New best Mopred: " << best << endl;
		}
		fann_destroy(ann);
	}

	cout << "writing to file" << endl;
	inFile.open ("../Shared/BAG/Bagging.data");

	for (i = 0; i < PERMS; ++i) // for each row
	{
		inFile << parabuff[i][0] << "\t" << parabuff[i][1] << endl;
	}
	inFile.close(); //close file

	cout << "Lowest BFEaccum: " << BFEleast << "  Best Mopred: " << best << endl;
	printf("Cleaning up.\n");
	fann_destroy_train(data);
	fann_destroy(ann);
	return ret;
}
