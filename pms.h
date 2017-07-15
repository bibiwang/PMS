#pragma once
#ifndef PMS_HEADER
#define PMS_HEADER

#include <iostream>
#include <opencv2/opencv.hpp>

#define FILENAME "rainy1.mp4"
#define ESC 27
#define DELAYMILL 20
#define PI 3.14159265
#define DEG2RAD PI/180

using namespace std;
using namespace cv;

Mat houghLine(Mat srcFrame) {
	//Exception handling (Image is Empty)
	if (srcFrame.empty() == true) {
		cout << "Frame is Empty" << endl;
	}

	Mat dstFrame, grayFrame;
	dstFrame = srcFrame.clone();
	cvtColor(srcFrame, grayFrame, COLOR_BGR2GRAY);

	//1. GaussianBular and Canny Edge Detection
	Mat edgeFrame;
	GaussianBlur(grayFrame, grayFrame, Size(9, 9), 1.0);
	Canny(grayFrame, edgeFrame, 100, 150);


	//2. Mapping of edge points to the Hough space and storage in an accumulator
	/*vector<Vec4i> lines;
	HoughLinesP()*/
	return edgeFrame;
}

#endif