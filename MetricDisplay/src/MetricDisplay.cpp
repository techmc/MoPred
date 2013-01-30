//============================================================================
// Name        : MetricDisplay.cpp
// Author      : Zassa Kavuma
// Version     : 0.2
// Copyright   : Released under the GNU General Public License
// Description : Program for Displaying Kinect Metrics
//============================================================================

// Includes
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <ni/XnCppWrapper.h>
#include <sstream>
#include <fstream> //test
#include <cstdio>
#include <ctime>

// Defines
#define PAIRS 64 // Sets the number of data pairs to record
#define PAIRSOUT ((PAIRS/2)+1) // Set the number of DFT output pairs
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

// Code

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


	//struct fann_train_data *data;
	//data = fann_read_train_from_file("../Shared/DCT000001.data"); // Import data
	//fann_save_train(data, "../Shared/DCT000001.data");

	fstream inFile;
	float parabuff[32][12];
	unsigned int i, j;
	inFile.open ("../Shared/dw.data"); //edit
	for (i = 0; i < 32; ++i) // for each row
	{
		for (j = 0; j < 12; ++j) // read each value into calculation array
		{
			inFile >> parabuff[i][j]; // Read file to row j column k
		}
	}
	inFile.close(); //close file

	cout << parabuff[31][11] << endl;

	inFile.open ("../Shared/DW.data"); //edit
	for (i = 0; i < 32; ++i) // for each row
	{
		for (j = 0; j < 12; ++j) // read each value into calculation array
		{
			inFile << parabuff[i][j] << "\t"; // Read file to row j column k
		}
		inFile << endl;
	}
	inFile.close(); //close file


	stringstream ss;
	string track;
	int x = 2;
	ss << x;
	track = ss.str();
	cout << endl << track << endl;
	//string basename = "../Shared/RAW0001dualwaveX.dat";
	//basename.replace(25, 1, track);
	//string basename = "../Shared/RAW0100dualriseX.dat";
	//basename.replace(25, 1, track);
	//string basename = "../Shared/RAW000010dualrowX.dat";
	//basename.replace(26, 1, track);
	//string basename = "../Shared/RAW000001timeX.dat";
	//basename.replace(23, 1, track);
	//string basename = "../Shared/RAW000010clapX.dat";
	//basename.replace(23, 1, track);
	//string basename = "../Shared/RAW010000throwX.dat";
	//basename.replace(24, 1, track);
	string basename = "../Shared/MopredX.net";
	basename.replace(16, 1, track);
	cout << basename << endl;

	unsigned int max_epochs;
	for (max_epochs = 100; max_epochs <= 1000; max_epochs=max_epochs+100)
	{
		cout << max_epochs << endl;
	}


	/*
	int iter=0; // paircount
	clock_t start;
	double diff;
	start = clock();

	while (iter<64) // pairs
	{
		diff = 50000;
		if (clock()==start+diff)
		{
			cout << iter << endl;

			start = clock();
			cout << clock() << endl;
			iter++;
		}
	}
	*/

	XnStatus nRetVal = XN_STATUS_OK;
	xn::Context context;
	nRetVal = context.Init();
	CHECK_RC( nRetVal, "Init" );

	// Create the user generator
	nRetVal = g_UserGenerator.Create(context);
	CHECK_RC( nRetVal, "Create" );
	XnCallbackHandle h1, h2, h3;
	g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, h1);
	// This is now PoseDetected, change?
	g_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks( Pose_Detected, NULL, NULL, h2);
	// This is now CalibrationStart and CalibrationComplete, change?
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

	int record = 1; // Record flag


	// Position floats
	float posit0lhx = 0;
	float posit0lsx = 0;
	float posit0lhy = 0;
	float posit0lsy = 0;
	float posit0lhz = 0;
	float posit0lsz = 0;

	float posit0rhx = 0;
	float posit0rsx = 0;
	float posit0rhy = 0;
	float posit0rsy = 0;
	float posit0rhz = 0;
	float posit0rsz = 0;

	// Polar floats
	float polarlhr = 0;
	float polarlhf = 0;
	float polarlvr = 0;
	float polarlvf = 0;

	float polarrhr = 0;
	float polarrhf = 0;
	float polarrvr = 0;
	float polarrvf = 0;

	while (TRUE)
	{
		// Update to next frame
		nRetVal = context.WaitAndUpdateAll();
		CHECK_RC( nRetVal, "WaitAndUpdateAll" );

		// Set number of users
		XnUserID aUsers[15]; //users
		XnUInt16 nUsers = 15; //users count
		g_UserGenerator.GetUsers(aUsers, nUsers);

		for (int i = 0; i < nUsers; ++i) //++i?
		{
			if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])) // If user is being tracked
			{
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_HAND, leftHand);
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_SHOULDER, leftShoulder);
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_RIGHT_HAND, rightHand);
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_RIGHT_SHOULDER, rightShoulder);

				if (record==1)
				{
					// Record positions
					posit0lhx = leftHand.position.X + 1024;
					posit0lhy = leftHand.position.Y + 1024;
					posit0lhz = leftHand.position.Z + 1024;
					posit0rhx = rightHand.position.X + 1024;
					posit0rhy = rightHand.position.Y + 1024;
					posit0rhz = rightHand.position.Z + 1024;

					posit0lsx = leftShoulder.position.X + 1024;
					posit0lsy = leftShoulder.position.Y + 1024;
					posit0lsz = leftShoulder.position.Z + 1024;
					posit0rsx = rightShoulder.position.X + 1024;
					posit0rsy = rightShoulder.position.Y + 1024;
					posit0rsz = rightShoulder.position.Z + 1024;

					// 1 Compute XZ polar radius
					polarlhr = sqrt(((posit0lhx-posit0lsx)*(posit0lhx-posit0lsx))+((posit0lhz-posit0lsz)*(posit0lhz-posit0lsz)));
					// 2 Compute XZ angle
					polarlhf = atan((posit0lhx-posit0lsx)/(posit0lhz-posit0lsz));
					// 3 Compute YZ polar radius
					polarlvr = sqrt(((posit0lhy-posit0lsy)*(posit0lhy-posit0lsy))+((posit0lhz-posit0lsz)*(posit0lhz-posit0lsz)));
					// 4 Compute YZ angle
					polarlvf = atan((posit0lhy-posit0lsy)/(posit0lhz-posit0lsz));

					// 5 Compute XZ polar radius
					polarrhr = sqrt(((posit0rhx-posit0rsx)*(posit0rhx-posit0rsx))+((posit0rhz-posit0rsz)*(posit0rhz-posit0rsz)));
					// 6 Compute XZ angle
					polarrhf = atan((posit0rhx-posit0rsx)/(posit0rhz-posit0rsz));
					// 7 Compute YZ polar radius
					polarrvr = sqrt(((posit0rhy-posit0rsy)*(posit0rhy-posit0rsy))+((posit0rhz-posit0rsz)*(posit0lhz-posit0rsz)));
					// 8 Compute YZ angle
					polarrvf = atan((posit0rhy-posit0rsy)/(posit0rhz-posit0rsz));

					// NEW METRICS

					// hand to hand distance

					// knee to shoulder distance

					// knee to knee distance


					//cout << posit0lhx << "\t" << posit0lhy << "\t" << posit0lhz << "\t" << posit0lsx << "\t"<< posit0lsy << "\t"<< posit0lsz << endl;
					cout << polarlhr << "	" << polarlhf << "	" << polarlvr << "	" << polarlvf;
					cout << polarrhr << "	" << polarrhf << "	" << polarrvr << "	" << polarrvf << endl;
				}
				else
				{
					cout << "exit flag set" << endl;
					return 0;
				}
			}
		}
	}

	// Clean up
	context.Release();
	cout<< "Goodbye" <<endl;
	return 0;
}
