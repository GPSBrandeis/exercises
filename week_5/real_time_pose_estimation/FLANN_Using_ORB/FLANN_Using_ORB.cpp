// FLANN_Using_ORB.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <time.h>

// OpenCV
#include <opencv2//core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/video/tracking.hpp>

#include "RobustMatcher.h"

using namespace cv;
using namespace std;

// Robust Matcher parameters
float ratioTest = 0.85f; //  0.70f;      // ratio test
bool fast_match = false;       // fastRobustMatch() or robustMatch()

const char* keys =
"{ help h |                          | Print help message. }"
"{ input1 | ../data/box.png          | Path to input image 1. }"
"{ input2 | ../data/box_in_scene.png | Path to input image 2. }";;



int main(int argc, char **argv) {
	// Read both images.
	cv::Mat image1 = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
	if (image1.empty()) {
		std::cerr << "Couldn't read image in " << argv[1] << std::endl;
		return 1;
	}
	cv::Mat image2 = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);
	if (image2.empty()) {
		std::cerr << "Couldn't read image in " << argv[2] << std::endl;
		return 1;
	}

	// Detect the keyPoints and compute its descriptors using ORB Detector.
	std::vector<cv::KeyPoint> keyPoints1, keyPoints2;
	cv::Mat descriptors1, descriptors2;
	cv::Ptr<cv::ORB> orb = cv::ORB::create();
	orb->detectAndCompute(image1, cv::Mat(), keyPoints1, descriptors1);
	orb->detectAndCompute(image2, cv::Mat(), keyPoints2, descriptors2);

	RobustMatcher rmatcher;                                               // instantiate RobustMatcher
	rmatcher.setFeatureDetector(orb);                                     // set feature detector
	rmatcher.setDescriptorExtractor(orb);                                 // set descriptor extractor

	Ptr<flann::IndexParams> indexParams = makePtr<flann::LshIndexParams>(6, 12, 1); // instantiate LSH index parameters
	Ptr<flann::SearchParams> searchParams = makePtr<flann::SearchParams>(50);       // instantiate flann search parameters

	// instantiate FlannBased matcher
	Ptr<DescriptorMatcher> matcher = makePtr<FlannBasedMatcher>(indexParams, searchParams);
	rmatcher.setDescriptorMatcher(matcher);                              // set matcher
	rmatcher.setRatio(ratioTest); // set ratio test parameter

	// Match features.
	std::vector<cv::DMatch> matches;
	vector<KeyPoint> keypoints_scene;  // to obtain the 2D points of the scene

	if (fast_match)
		rmatcher.fastRobustMatch(image1, matches, keypoints_scene, descriptors1);
	else
		rmatcher.robustMatch(image1, matches, keypoints_scene, descriptors1);

	// Draw matches.
	cv::Mat image_matches;
	cv::drawMatches(image1, keyPoints1, image2, keyPoints2, matches, image_matches);
	cv::imshow("Matches", image_matches);
	waitKey();
	return 0;
}