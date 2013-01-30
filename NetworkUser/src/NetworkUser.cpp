//============================================================================
// Name        : NetworkUser.cpp
// Author      : Zassa Kavuma
// Version     : 0.1
// Copyright   : Released under the GNU General Public License
// Description : Program for evaluating neural network performance
//============================================================================

// TODO: Take random sample of recorded data
// TODO: Test performance of Neural Network against sample
// TODO: Record the performance data
// Could also have a real time performance evaluation (maybe separate function)

// include maths?

/*
#include <iostream>
#include <fstream>
#include <GL/glut.h>
#include <ni/XnCppWrapper.h>
*/

#include <stdio.h>
#include "fann.h"

int main()
{
	fann_type *calc_out;
	unsigned int i;
	int ret = 0;

	struct fann *ann;
	struct fann_train_data *data;

	printf("Creating network.\n");

#ifdef FIXEDFANN
	ann = fann_create_from_file("../Shared/handtrain1_fixed.net");
#else
	ann = fann_create_from_file("../Shared/handtrain1_float.net");
#endif

	if(!ann)
	{
		printf("Error creating ann --- ABORTING.\n");
		return 0;
	}

	fann_print_connections(ann);
	fann_print_parameters(ann);

	printf("Testing network.\n");

#ifdef FIXEDFANN
	data = fann_read_train_from_file("../Shared/handtrain1_fixed.data");
#else
	data = fann_read_train_from_file("../Shared/handtrain1.data");
#endif

	for(i = 0; i < fann_length_train_data(data); i++)
	{
		fann_reset_MSE(ann);
		calc_out = fann_test(ann, data->input[i], data->output[i]);
#ifdef FIXEDFANN
		printf("XOR test (%d, %d) -> %d, should be %d, difference=%f\n",
			   data->input[i][0], data->input[i][1], calc_out[0], data->output[i][0],
			   (float) fann_abs(calc_out[0] - data->output[i][0]) / fann_get_multiplier(ann));

		if((float) fann_abs(calc_out[0] - data->output[i][0]) / fann_get_multiplier(ann) > 0.2)
		{
			printf("Test failed\n");
			ret = -1;
		}
#else
		printf("XOR test (%f, %f) -> %f, should be %f, difference=%f\n",
			   data->input[i][0], data->input[i][1], calc_out[0], data->output[i][0],
			   (float) fann_abs(calc_out[0] - data->output[i][0]));
#endif
	}

	printf("Cleaning up.\n");
	fann_destroy_train(data);
	fann_destroy(ann);

	return ret;
}



/*
using namespace xn;
using namespace std;

#define window_width  640
#define window_height 480
#define PAIRS 50 // sets the number of data pairs to record
#define CHECK_RC( nRetVal, issue )                                          \
    if( nRetVal != XN_STATUS_OK )                                           \
    {                                                                       \
        printf( "%s failed: %s\n", issue, xnGetStatusString( nRetVal ) );   \
        return nRetVal;                                                     \
    }
#define POSE_TO_USE "Psi"
xn::UserGenerator g_UserGenerator;

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

// Glut Main loop
void main_loop_function() // passed to glutIdle
{
	// Clear color (screen)
	// And depth (used internally to block obstructed objects)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load identity matrix
	glLoadIdentity();
	// Multiply in translation matrix
	glTranslatef(0,0, -10);
	// Render colored quad
	glBegin(GL_QUADS);
	glColor3ub(255, 255, 000); glVertex2f(-0.5,  0.5);
	glColor3ub(255, 255, 000); glVertex2f( 1,  1);
	glColor3ub(255, 255, 000); glVertex2f( 0.5, -0.5);
	glColor3ub(255, 255, 000); glVertex2f(-1, -1);
	glEnd();
	// Swap buffers (color buffers, makes previous render visible)
	glutSwapBuffers();
}

// Initialze OpenGL perspective matrix
void GL_Setup(int width, int height)
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glEnable( GL_DEPTH_TEST );
	gluPerspective( 45, (float)width/height, .1, 100 );
	glMatrixMode( GL_MODELVIEW );
}


int main(int argc, char** argv) // argument count, argument vector
{
	// Declarations
	int paircount = 0;
	int delaycount = 0;
	int arrayloaddelay = 0; // delays writing to file until first delta values are calculated

	float positfx = 0;
	float posit0x = 0;
	float posit1x = 0;
	float posit2x = 0;
	float delta1x = 0;
	float delta2x = 0;
	float deltafx = 0;

	printf("Welcome Trainer!\n");

	// Initialize data file
	ofstream myfile;
	myfile.open ("../Shared/handtrain1.data");
	myfile << PAIRS <<" 2 1" << "\n";
	myfile.close();

	XnStatus nRetVal = XN_STATUS_OK; // set return value for error checking
	xn::Context context;
	nRetVal = context.Init();
	CHECK_RC( nRetVal, "Init" );

	// Create the user generator
	nRetVal = g_UserGenerator.Create(context);
	CHECK_RC( nRetVal, "Create(context)" );
	XnCallbackHandle h1, h2, h3;
	g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, h1);
	g_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks( Pose_Detected, NULL, NULL, h2);
	g_UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks( Calibration_Start, Calibration_End, NULL, h3);
	// Set the profile
	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
	// Start generating
	nRetVal = context.StartGeneratingAll();
	// TODO: check error code

	// Initialize GLUT and start main loop
	//glutInit(&argc, argv);
	//glutInitWindowSize(window_width, window_height);
	//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	//glutCreateWindow("Badman Display!");
	//glutIdleFunc(main_loop_function);
	//GL_Setup(window_width, window_height);

	while (TRUE)
	{
		// Update to next frame
		nRetVal = context.WaitAndUpdateAll();
		// TODO: check error code

		// Extract user positions of each tracked user
		XnUserID aUsers[15];
		XnUInt16 nUsers = 15;
		g_UserGenerator.GetUsers(aUsers, nUsers);
		for (int i = 0; i < nUsers; ++i)
		{
			if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])) // if user is being tracked
			{
				//XnSkeletonJointPosition Head;
				//g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_HEAD, Head);

				XnSkeletonJointPosition leftHand;
				g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[i], XN_SKEL_LEFT_HAND, leftHand);
				printf("%d: (%f,%f,%f) [%f]\n", aUsers[i],	leftHand.position.X, leftHand.position.Y, leftHand.position.Z,	leftHand.fConfidence);

				if (paircount<PAIRS) // take a set amount of readings
				{
					if (delaycount==5) // delay between readings
					{
						if (arrayloaddelay>5) // compute data and write to file when array* full
						{
							// open data file for appending
							myfile.open ("../Shared/handtrain1.data",ios::app);

							// compute new data, this works for equal time spacing
							positfx = leftHand.position.X + 1024;
							deltafx = positfx - posit0x;
							delta1x = posit0x - posit1x;
							delta2x = posit0x - posit2x;

							// write data to file
							myfile << delta2x << " " << delta1x << "\n" << deltafx <<"\n";

							// close data file
							myfile.close();

							// prepare data for next iteration
							posit2x = posit1x;
							posit1x = posit0x;
							posit0x = positfx;

							delaycount=0;
							paircount++;
						}
						else // compute data only
						{
							// compute new data, this works for equal time spacing
							positfx = leftHand.position.X + 1024;
							deltafx = positfx - posit0x;
							delta1x = posit0x - posit1x;
							delta2x = posit0x - posit2x;

							// prepare data for next iteration
							posit2x = posit1x;
							posit1x = posit0x;
							posit0x = positfx;

							delaycount=0;
							arrayloaddelay++;
						}
					}
					else
					{
						delaycount++;
					}
				}
				else
				{
					context.Release();
					cout<< "Thanks for the data, Trainer. Goodbye!" <<endl;
					return 0;
				}
			}
		}
	}

	//glutMainLoop();

	// Clean up
	context.Release();
	cout<< "Goodbye" <<endl;
	return 0;
}
*/
