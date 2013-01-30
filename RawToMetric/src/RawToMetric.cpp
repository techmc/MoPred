//============================================================================
// Name        : RawToMetric.cpp
// Author      : Zassa Kavuma
// Version     : 0.2
// Copyright   : Released under the GNU General Public License
// Description : Program for saving metric data
//============================================================================

// Includes
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <fftw3.h>
#include <fann.h>
#include <sstream>

// Defines
#define FILES 	60 // max number of files
#define PAIRS 	32 // Sets the number of data pairs to record
#define PAIRSOUT ((PAIRS/2)+1) // Set the number of DFT output pairs
#define VALUES 	12
#define POLARS 	8
#define DAT 	3 // sets the current DCT file

// Globals
using namespace std;

int main(int argc, char** argv)
{

	fftw_complex    *dctdata, *fft_result;
	fftw_plan       plan_forward;
	stringstream 	ss;
	string 			track;
	ifstream 		inFile;

	dctdata        	= ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * PAIRS );
	fft_result  	= ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * PAIRS );
	plan_forward 	= fftw_plan_dft_1d( PAIRS, dctdata, fft_result, FFTW_FORWARD, FFTW_ESTIMATE ); // Position floats (12)

	int             i;
	
	// Buffers
	float 			positbuffer[PAIRS][VALUES]; 
	double 			polarbuffer[PAIRS][POLARS];

	printf("Welcome Trainer!\n");

	for (int x = 0; x < FILES; ++x) // For each data file
	{
			ss << x;
			track = ss.str();
			cout << track << endl;
			string basename;

			if (DAT==1){
				basename = "../Shared/RAW/RAW000001dualwaveX.dat";
				basename.replace(27, 1, track);
			}
			if (DAT==2){
				basename = "../Shared/RAW/RAW000010dualrowX.dat";
				basename.replace(26, 1, track);
			}
			if (DAT==3){
				basename = "../Shared/RAW/RAW000100dualriseX.dat";
				basename.replace(27, 1, track);
			}
			if (DAT==4){
				basename = "../Shared/RAW/RAW001000timeX.dat";
				basename.replace(23, 1, track);
			}
			if (DAT==5){
				basename = "../Shared/RAW/RAW010000clapX.dat";
				basename.replace(23, 1, track);
			}
			if (DAT==6){
				basename = "../Shared/RAW/RAW100000crossX.dat";
				basename.replace(24, 1, track);
			}

			cout << basename << endl;
			ss.str("");

			// open file if it exists
			ifstream fp(basename.c_str());
			if (fp)
			{
				inFile.open(basename.c_str());
				cout << "Entering polar area" << endl;

				for (int j = 0; j < PAIRS; ++j) // for each row
				{
					// Convert row of values into polar values
					for (int k = 0; k < VALUES; ++k) // read each value into calculation array
					{
						inFile >> positbuffer[j][k]; // Read file to row j column k
					}

					// 1 Compute XZ polar radius
					polarbuffer[0][j] = sqrt(((positbuffer[j][0]-positbuffer[j][6])*(positbuffer[j][0]-positbuffer[j][6]))+((positbuffer[j][5]-positbuffer[j][8])*(positbuffer[j][2]-positbuffer[j][8])));
					// 2 Compute XZ angle
					polarbuffer[1][j] = atan((positbuffer[j][0]-positbuffer[j][6])/(positbuffer[j][2]-positbuffer[j][8]));
					// 3 Compute YZ polar radius
					polarbuffer[2][j] = sqrt(((positbuffer[j][1]-positbuffer[j][7])*(positbuffer[j][1]-positbuffer[j][7]))+((positbuffer[j][2]-positbuffer[j][8])*(positbuffer[j][2]-positbuffer[j][8])));
					// 4 Compute YZ angle
					polarbuffer[3][j] = atan((positbuffer[j][1]-positbuffer[j][7])/(positbuffer[j][2]-positbuffer[j][8]));
					// 5 Compute XZ polar radius
					polarbuffer[4][j] = sqrt(((positbuffer[j][3]-positbuffer[j][9])*(positbuffer[j][3]-positbuffer[j][9]))+((positbuffer[j][5]-positbuffer[j][11])*(positbuffer[j][5]-positbuffer[j][11])));
					// 6 Compute XZ angle
					polarbuffer[5][j] = atan((positbuffer[j][3]-positbuffer[j][9])/(positbuffer[j][5]-positbuffer[j][11]));
					// 7 Compute YZ polar radius
					polarbuffer[6][j] = sqrt(((positbuffer[j][4]-positbuffer[j][10])*(positbuffer[j][4]-positbuffer[j][10]))+((positbuffer[j][5]-positbuffer[j][11])*(positbuffer[j][2]-positbuffer[j][11])));
					// 8 Compute YZ angle
					polarbuffer[7][j] = atan((positbuffer[j][4]-positbuffer[j][10])/(positbuffer[j][5]-positbuffer[j][11]));

				}
				inFile.close();

				// DCT computation
				cout << "Entering DCT area" << endl;
				ofstream myfile2;
				if (DAT==1){
					myfile2.open ("../Shared/DCT/DCT000001.data", ios::app);
				}
				if (DAT==2){
					myfile2.open ("../Shared/DCT/DCT000010.data", ios::app);
				}
				if (DAT==3){
					myfile2.open ("../Shared/DCT/DCT000100.data", ios::app);
				}
				if (DAT==4){
					myfile2.open ("../Shared/DCT/DCT001000.data", ios::app);
				}
				if (DAT==5){
					myfile2.open ("../Shared/DCT/DCT010000.data", ios::app);
				}
				if (DAT==6){
					myfile2.open ("../Shared/DCT/DCT100000.data", ios::app);
				}

				float 	absmat[PAIRSOUT]; // Array to hold absolute outputs
				int 	a;

				// 1 XZ radius
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[0][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){ // Save the 5 largest values
					myfile2 << absmat[a]  << " ";
				}
				// 2 XZ angle
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[1][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}
				// 3 YZ radius
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[2][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}
				// 4 YZ angle
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[3][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}
				// 5
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[4][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}
				// 6
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[5][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}
				// 7
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[6][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}
				// 8
				for( i = 0 ; i < PAIRS ; i++ ) {
					dctdata[i][0] = polarbuffer[7][i];
					dctdata[i][1] = 0.0;
				}
				fftw_execute( plan_forward );
				for (a=0; a<PAIRSOUT; a++){
					absmat[a] = sqrt(((fft_result[a][0])*(fft_result[a][0]))+((fft_result[a][1])*(fft_result[a][1])));
				}
				sort(absmat, absmat+PAIRSOUT);
				for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
					myfile2 << absmat[a]  << " ";
				}

				// Write the binary classifier
				if (DAT==1){
					myfile2 << "\n" << "-1 -1 -1 -1 -1 1" << "\n";
				}
				if (DAT==2){
					myfile2 << "\n" << "-1 -1 -1 -1 1 -1" << "\n";
				}
				if (DAT==3){
					myfile2 << "\n" << "-1 -1 -1 1 -1 -1" << "\n";
				}
				if (DAT==4){
					myfile2 << "\n" << "-1 -1 1 -1 -1 -1" << "\n";
				}
				if (DAT==5){
					myfile2 << "\n" << "-1 1 -1 -1 -1 -1" << "\n";
				}
				if (DAT==6){
					myfile2 << "\n" << "1 -1 -1 -1 -1 -1" << "\n";
				}

				cout << "Leaving DCT area \n" << endl;
				// Close data file
				myfile2.close();
			}
			else
			{
				cout << "File not found \n" << endl;
			}
		}
	// Shuffle file data
	struct fann_train_data *data;
	if (DAT==1){
		data = fann_read_train_from_file("../Shared/DCT/DCT000001.data"); // Import data
		remove("../Shared/DCT/DCT000001.data");
		fann_shuffle_train_data(data); // Shuffle data
		fann_save_train(data, "../Shared/DCT/DCT000001.data");
	}
	if (DAT==2){
		data = fann_read_train_from_file("../Shared/DCT/DCT000010.data"); // Import data
		remove("../Shared/DCT/DCT000010.data");
		fann_shuffle_train_data(data); // Shuffle data
		fann_save_train(data, "../Shared/DCT/DCT000010.data");
	}
	if (DAT==3){
		data = fann_read_train_from_file("../Shared/DCT/DCT000100.data"); // Import data
		remove("../Shared/DCT/DCT000100.data");
		fann_shuffle_train_data(data); // Shuffle data
		fann_save_train(data, "../Shared/DCT/DCT000100.data");
	}
	if (DAT==4){
		data = fann_read_train_from_file("../Shared/DCT/DCT001000.data"); // Import data
		remove("../Shared/DCT/DCT001000.data");
		fann_shuffle_train_data(data); // Shuffle data
		fann_save_train(data, "../Shared/DCT/DCT001000.data");
	}
	if (DAT==5){
		data = fann_read_train_from_file("../Shared/DCT/DCT010000.data"); // Import data
		remove("../Shared/DCT/DCT010000.data");
		fann_shuffle_train_data(data); // Shuffle data
		fann_save_train(data, "../Shared/DCT/DCT010000.data");
	}
	if (DAT==6){
		data = fann_read_train_from_file("../Shared/DCT/DCT100000.data"); // Import data
		remove("../Shared/DCT/DCT100000.data");
		fann_shuffle_train_data(data); // Shuffle data
		fann_save_train(data, "../Shared/DCT/DCT100000.data");
	}

	// Free memory
	fftw_free( dctdata );
	fftw_free( fft_result );
	fftw_destroy_plan( plan_forward );
	cout<< "Computations are complete. DCT file created, Goodbye!" <<endl;
	return 0;

}

