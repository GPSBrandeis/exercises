// bgfg_Segmentation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.


#include "pch.h"
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering


using namespace std;
using namespace cv;
using namespace rs2;

int main(int argc, const char** argv)
{
	rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);

	const String keys = "{fn file_name | | use video file as input }"
		"{m method | mog2 | method: background subtraction algorithm ('knn', 'mog2')}"
		"{h help | | show help message}";
	CommandLineParser parser(argc, argv, keys);
	parser.about("This sample demonstrates background segmentation.");
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}
	String file = parser.get<String>("file_name");
	String method = parser.get<String>("method");
	if (!parser.check())
	{
		parser.printErrors();
		return 1;
	}

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;
	// Start streaming with default recommended configuration
	pipe.start();

	Ptr<BackgroundSubtractor> model;
	if (method == "knn")
		model = createBackgroundSubtractorKNN();
	else if (method == "mog2")
		model = createBackgroundSubtractorMOG2();
	if (!model)
	{
		cout << "Can not create background model using provided method: '" << method << "'" << endl;
		return 3;
	}

	cout << "Press <space> to toggle background model update" << endl;
	cout << "Press 's' to toggle foreground mask smoothing" << endl;
	cout << "Press ESC or 'q' to exit" << endl;
	bool doUpdateModel = true;
	bool doSmoothMask = false;

	Mat frame, foregroundMask, foreground, background;
	for (;;)
	{
		rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
		rs2::frame color = data.get_color_frame();            // Find the color data

		// Query frame size (width and height)
		const int w = color.as<rs2::video_frame>().get_width();
		const int h = color.as<rs2::video_frame>().get_height();

		// Create OpenCV matrix of size (w,h) from the colorized depth data
		Mat inputFrame(Size(w, h), CV_8UC3, (void*)color.get_data(), Mat::AUTO_STEP);


		const Size scaledSize(640, 640 * inputFrame.rows / inputFrame.cols);
		resize(inputFrame, frame, scaledSize, 0, 0, INTER_LINEAR);

		// pass the frame to background model
		model->apply(frame, foregroundMask, doUpdateModel ? -1 : 0);

		// show processed frame
		imshow("image", frame);

		// show foreground image and mask (with optional smoothing)
		if (doSmoothMask)
		{
			GaussianBlur(foregroundMask, foregroundMask, Size(11, 11), 3.5, 3.5);
			threshold(foregroundMask, foregroundMask, 10, 255, THRESH_BINARY);
		}
		if (foreground.empty())
			foreground.create(scaledSize, frame.type());
		foreground = Scalar::all(0);
		frame.copyTo(foreground, foregroundMask);
		imshow("foreground mask", foregroundMask);
		imshow("foreground image", foreground);

		// show background image
		model->getBackgroundImage(background);
		if (!background.empty())
			imshow("mean background image", background);

		// interact with user
		const char key = (char)waitKey(30);
		if (key == 27 || key == 'q') // ESC
		{
			cout << "Exit requested" << endl;
			break;
		}
		else if (key == ' ')
		{
			doUpdateModel = !doUpdateModel;
			cout << "Toggle background update: " << (doUpdateModel ? "ON" : "OFF") << endl;
		}
		else if (key == 's')
		{
			doSmoothMask = !doSmoothMask;
			cout << "Toggle foreground mask smoothing: " << (doSmoothMask ? "ON" : "OFF") << endl;
		}
	}
	return EXIT_SUCCESS;
}
