//============================================================================
// Name        : DataSaver.cpp
// Author      : Zassa Kavuma
// Version     : 0.2
// Copyright   : Released under the GNU General Public License
// Description : Program for saving joint position data
//============================================================================

// Includes
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <ni/XnCppWrapper.h>
#include <fftw3.h>
#include <sstream>
#include <cstdio>
#include <ctime>

// Defines
#define FILES 	60
#define PAIRS 	32 // Sets the number of data pairs to record
#define PAIRSOUT ((PAIRS/2)+1) // Set the number of DFT output pairs
#define VALUES 	12
#define DAT 	1
#define POSE_TO_USE "Psi"
#define CHECK_RC( nRetVal, issue )                                          \
    if( nRetVal != XN_STATUS_OK )                                           \
    {                                                                       \
        printf( "%s failed: %s\n", issue, xnGetStatusString( nRetVal ) );   \
        return nRetVal;                                                     \
    }

// Globals
xn::UserGenerator g_UserGenerator;
using namespace xn;
using namespace std;

// Callbacks

void XN_CALLBACK_TYPE
User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie){
	printf("New User: %d\n", nId);
	g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(POSE_TO_USE,
	nId);
}
void XN_CALLBACK_TYPE
User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{}
void XN_CALLBACK_TYPE
Pose_Detected(xn::PoseDetectionCapability& pose, const XnChar* strPose, XnUserID nId, void* pCookie){
	printf("Pose %s for user %d\n", strPose, nId);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
void XN_CALLBACK_TYPE
Calibration_Start(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie){
	printf("Starting calibration for user %d\n", nId);
}
void XN_CALLBACK_TYPE
Calibration_End(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie){
	if (bSuccess){
		printf("User calibrated\n");
		g_UserGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else{
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
	g_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks( Pose_Detected, NULL, NULL, h2); // undeprecate?
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

	stringstream 	ss;
	string 			track;
	ifstream 		inFile;

	// Check for previous files and create new file
	for (int x = 0; x < FILES; ++x) // for each raw data file
	{
		ss << x;
		track = ss.str();
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
			basename = "../Shared/RAW/RAW001000timeX.dat"; //hmm
			basename.replace(23, 1, track);
		}
		if (DAT==5){
			basename = "../Shared/RAW/RAW010000clapX.dat";
			basename.replace(23, 1, track);
		}
		if (DAT==6){
			basename = "../Shared/RAW/RAW100000crossX.dat"; //hmm
			basename.replace(24, 1, track);
		}

		cout << basename << endl;
		ss.str("");

		ifstream fp(basename.c_str());
		if (fp==NULL) // if file does not exist
		{
			// Open data file for appending
			ofstream myfile1;
			myfile1.open (basename.c_str(),ios::app);

			int paircount = 0;
			int memwrite = 0;
			int mw;
			float positbuffer[PAIRS][VALUES]; // Buffer used to have only one write to disk event
			
			printf("Welcome Trainer!\n");
			while (TRUE)
			{
				// Update to next frame
				nRetVal = context.WaitAndUpdateAll();
				CHECK_RC( nRetVal, "WaitAndUpdateAll" );

				// Set number of users
				XnUserID aUsers[15]; //users
				XnUInt16 nUsers = 15; //users count
				g_UserGenerator.GetUsers(aUsers, nUsers);

				for (int i = 0; i < 1; ++i) // For defined users (2)
				{
					if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])) // If user is being tracked
					{
						g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_HAND, leftHand);
						g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_SHOULDER, leftShoulder);
						g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_RIGHT_HAND, rightHand);
						g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_RIGHT_SHOULDER, rightShoulder);

						clock_t start;
						double diff = 50000;

						if (paircount<PAIRS) // Take a number of readings set by PAIRS
						{
							if (clock()>=start+diff)
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

								// Write to file when buffer full
								if (memwrite==(PAIRS-1)) // When recording complete
								{
									// Write positions to buffer file
									for (mw=0; mw<PAIRS; mw++)
										myfile1 << positbuffer[mw][0] << "\t" << positbuffer[mw][1] << "\t" << positbuffer[mw][2] << "\t" << positbuffer[mw][3] << "\t"
												<< positbuffer[mw][4] << "\t" << positbuffer[mw][5] << "\t" << positbuffer[mw][6] << "\t" << positbuffer[mw][7] << "\t"
												<< positbuffer[mw][8] << "\t" << positbuffer[mw][9] << "\t" << positbuffer[mw][10] << "\t" << positbuffer[mw][11] << endl;
									// Close data file
									myfile1.close();
									memwrite=0;
								}
								else
								{
									memwrite++;
								}
								paircount++;
								start = clock(); // reset clock start
							}
						}
						else
						{
						context.Release();
						cout<< "Thanks for the data, Trainer!" <<endl;
						return 0;
						}
					}
				}
			}
			return 0;
		}
	}
	// Clean up
	context.Release();
	cout<< "Goodbye" <<endl;
	return 0;
}
