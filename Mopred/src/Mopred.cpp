//============================================================================
// Name        : DataSaver.cpp
// Author      : Zassa Kavuma
// Version     : 0.2
// Copyright   : Released under the GNU General Public License
// Description : Program for running a neural network in real time
//============================================================================

// Includes
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <floatfann.h>
#include <fftw3.h>
#include <ni/XnCppWrapper.h>
#include <cstdio>
#include <ctime>

// Defines
#define PAIRS 32 // Sets the number of data pairs to record
#define PAIRSOUT ((PAIRS/2)+1) // Set the number of DFT output pairs
#define VALUES 12
#define POLARS 8
#define DCTS (5*POLARS) // (40) Number of saved dct values
#define CHECK_RC( nRetVal, issue )                                          \
    if( nRetVal != XN_STATUS_OK )                                           \
    {                                                                       \
        printf( "%s failed: %s\n", issue, xnGetStatusString( nRetVal ) );   \
        return nRetVal;                                                     \
    }
#define POSE_TO_USE "Psi"

// Globals
xn::UserGenerator g_UserGenerator;
using namespace xn;
using namespace std;

void XN_CALLBACK_TYPE
User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	printf("New User: %d\n", nId);
	g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(POSE_TO_USE,
	nId);
}
void XN_CALLBACK_TYPE
User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{}
void XN_CALLBACK_TYPE
Pose_Detected(xn::PoseDetectionCapability& pose, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	printf("Pose %s for user %d\n", strPose, nId);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
void XN_CALLBACK_TYPE
Calibration_Start(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	printf("Starting calibration for user %d\n", nId);
}
void XN_CALLBACK_TYPE
Calibration_End(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	if (bSuccess)
	{
		printf("User calibrated\n");
		g_UserGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else
	{
		printf("Failed to calibrate user %d\n", nId);
		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(POSE_TO_USE, nId);
	}
}

int main(int argc, char** argv)
{
	// Declarations
	XnStatus nRetVal = XN_STATUS_OK;
	xn::Context context;
	nRetVal = context.Init();
	CHECK_RC( nRetVal, "Init" );

	// Create the user generator
	nRetVal = g_UserGenerator.Create(context);
	CHECK_RC( nRetVal, "Create" );
	XnCallbackHandle h1, h2, h3;
	g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, h1);
	g_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks( Pose_Detected, NULL, NULL, h2);
	g_UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks( Calibration_Start, Calibration_End, NULL, h3);
	// Set the profile
	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
	// Start generating
	nRetVal = context.StartGeneratingAll();
	CHECK_RC( nRetVal, "StartGeneratingAll" );

	XnSkeletonJointPosition leftHand;
	XnSkeletonJointPosition leftShoulder;
	XnSkeletonJointPosition rightHand;
	XnSkeletonJointPosition rightShoulder;

	struct fann 	*ann = fann_create_from_file("../Shared/Mopred.net");
	fftw_complex    *dctdata;
	fftw_complex    *fft_result;
	fftw_plan       plan_forward;
	fann_type 		input[DCTS];
	fann_type 		*calc_out;
	int record = 1; // Record flag
	int memwrite = 0;
	int incnt = 0; // NN input array counter

	// Delta buffer
	float positbuffer[PAIRS][VALUES];
	double polarbuffer[PAIRS][POLARS];

	printf("Welcome User!\n");
	clock_t start = clock();
	while (TRUE)
	{
		// Update to next frame
		nRetVal = context.WaitAndUpdateAll();
		CHECK_RC( nRetVal, "WaitAndUpdateAll" );
		// Set number of users
		XnUserID aUsers[15]; //users
		XnUInt16 nUsers = 15; //users count
		g_UserGenerator.GetUsers(aUsers, nUsers);

		for (int i = 0; i < 1; ++i) // for defined users (2)
		{
			if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])) // If user is being tracked
			{
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_HAND, leftHand);
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_SHOULDER, leftShoulder);
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_RIGHT_HAND, rightHand);
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_RIGHT_SHOULDER, rightShoulder);

				//clock_t start;
				double diff = 50000;
				
				if (record==1) // record until exit initiated
				{
					if (clock()>=start+diff) // If time difference is big enough
					{
						// Record positions
						positbuffer[memwrite][0] = leftHand.position.X + 1024;
						positbuffer[memwrite][1] = leftHand.position.Y + 1024;
						positbuffer[memwrite][2] = leftHand.position.Z + 1024;
						positbuffer[memwrite][3] = rightHand.position.X + 1024;
						positbuffer[memwrite][4] = rightHand.position.Y + 1024;
						positbuffer[memwrite][5] = rightHand.position.Z + 1024;
						positbuffer[memwrite][6] = leftShoulder.position.X + 1024;
						positbuffer[memwrite][7] = leftShoulder.position.Y + 1024;
						positbuffer[memwrite][8] = leftShoulder.position.Z + 1024;
						positbuffer[memwrite][9] = rightShoulder.position.X + 1024;
						positbuffer[memwrite][10] = rightShoulder.position.Y + 1024;
						positbuffer[memwrite][11] = rightShoulder.position.Z + 1024;

						// 1 Compute XZ polar radius
						polarbuffer[0][memwrite] = sqrt(((positbuffer[memwrite][0]-positbuffer[memwrite][6])*(positbuffer[memwrite][0]-positbuffer[memwrite][6]))+((positbuffer[memwrite][5]-positbuffer[memwrite][8])*(positbuffer[memwrite][2]-positbuffer[memwrite][8])));
						// 2 Compute XZ angle
						polarbuffer[1][memwrite] = atan((positbuffer[memwrite][0]-positbuffer[memwrite][6])/(positbuffer[memwrite][2]-positbuffer[memwrite][8]));
						// 3 Compute YZ polar radius
						polarbuffer[2][memwrite] = sqrt(((positbuffer[memwrite][1]-positbuffer[memwrite][7])*(positbuffer[memwrite][1]-positbuffer[memwrite][7]))+((positbuffer[memwrite][2]-positbuffer[memwrite][8])*(positbuffer[memwrite][2]-positbuffer[memwrite][8])));
						// 4 Compute YZ angle
						polarbuffer[3][memwrite] = atan((positbuffer[memwrite][1]-positbuffer[memwrite][7])/(positbuffer[memwrite][2]-positbuffer[memwrite][8]));
						// 5 Compute XZ polar radius
						polarbuffer[4][memwrite] = sqrt(((positbuffer[memwrite][3]-positbuffer[memwrite][9])*(positbuffer[memwrite][3]-positbuffer[memwrite][9]))+((positbuffer[memwrite][5]-positbuffer[memwrite][11])*(positbuffer[memwrite][5]-positbuffer[memwrite][11])));
						// 6 Compute XZ angle
						polarbuffer[5][memwrite] = atan((positbuffer[memwrite][3]-positbuffer[memwrite][9])/(positbuffer[memwrite][5]-positbuffer[memwrite][11]));
						// 7 Compute YZ polar radius
						polarbuffer[6][memwrite] = sqrt(((positbuffer[memwrite][4]-positbuffer[memwrite][10])*(positbuffer[memwrite][4]-positbuffer[memwrite][10]))+((positbuffer[memwrite][5]-positbuffer[memwrite][11])*(positbuffer[memwrite][2]-positbuffer[memwrite][11])));
						// 8 Compute YZ angle
						polarbuffer[7][memwrite] = atan((positbuffer[memwrite][4]-positbuffer[memwrite][10])/(positbuffer[memwrite][5]-positbuffer[memwrite][11]));

						unsigned int active_cnt;
						if (memwrite==(PAIRS-1)) // After every X iterations classify motion
						{
							// DCT
							// Create data structures
							dctdata     	= ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * PAIRS );
							fft_result  	= ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * PAIRS );
							plan_forward  	= fftw_plan_dft_1d( PAIRS, dctdata, fft_result, FFTW_FORWARD, FFTW_ESTIMATE );

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
							for (a=(PAIRSOUT-5); a<PAIRSOUT; a++){
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
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
								input[incnt] = absmat[a];
								incnt++;
							}

							incnt=0; // Reset NN input counter

							// NN
							float big = -1000000, little = 1000000;
							for( i = 0 ; i < DCTS ; i++ ) {
								if (big < input[i]) big = input[i]; // Find largest value
								if (little > input[i]) little = input[i]; // Find smallest value
							}
							// Scale input data
							for( i = 0 ; i < DCTS ; i++ ) {
								input[i]=(input[i]-little)/(big-little);
							}

							// Run network
							calc_out = fann_run(ann, input); // Also outputs MSE/BFE

							// Display ANN class prediction
							active_cnt=0;
							for (i=0; i<6; i++) // calculate N-of-many
							{
								if (calc_out[i]>0)
								{
									active_cnt++;
								}
							}

							if (active_cnt>1) // if 2-of-many
							{
								cout << " Unknown Type 1 \n\n";
							}
							else // 0or1-of-many
							{
								if (calc_out[0]>0) // 1XXXXX
								{
									cout << " Cross \n\n"; //
								}
								else // 0XXXXX
								{
									if (calc_out[1]>0) // 01XXXX
									{
										cout << " Clap \n\n ";
									}
									else // 00XXXX
									{
										if (calc_out[2]>0) // 001XXX
										{
											cout << " Timeout \n\n ";
										}
										else // 000XXXX
										{
											if (calc_out[3]>0) // 0001XX
											{
												cout << " Dual Rise\n\n ";
											}
											else // 0000XX
											{
												if (calc_out[4]>0) // 00001X
												{
													cout << " Dual Row\n\n ";
												}
												else // 00000X
												{
													if (calc_out[5]>0) // 000001
													{
														cout << " Dual Wave\n\n ";
													}
													else // 000000 0-of-many
													{
														cout << " Unknown Type 2\n\n ";
													}
												}
											}
										}
									}
								}
							}
							cout << "  Starting a new recording...\n\n";
							memwrite=0;
						}
						else // if data set not full
						{
							memwrite++;
						}
						start = clock(); // reset timer
					}
				}
				else // if record != 1
				{
					cout<< "Exit Initiated \n" <<endl;
					// Free memory
					fftw_destroy_plan( plan_forward );
					fftw_free( dctdata );
					fftw_free( fft_result );
					fann_destroy(ann);
					return 0;
				}
			}
		}
	}
	// Clean up
	context.Release();
	cout<< "Thankyou for using Mopred, I hope I was right. Goodbye!" <<endl;
	return 0;
}
