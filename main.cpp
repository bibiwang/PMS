#include <iostream>
#include <opencv2/opencv.hpp>
#include "pms.h"

using namespace std;
using namespace cv;

//DO NOT REVISE THIS MAIN CODE 
int main(void){

	VideoCapture video(FILENAME);

	//Video is not open
	if (!video.isOpened())
		cout << "Could not open video file." << endl;
	

	//VIDEO 320 X 240 
	video.set(CAP_PROP_FRAME_WIDTH, 320);
	video.set(CAP_PROP_FRAME_HEIGHT, 240);

	Mat frame, houghFrame;

	//Main Loop
	while(true){ 
		video >> frame;

		houghFrame = houghLine(frame);
		imshow("PMS", houghFrame);

		if (waitKey(DELAYMILL) == ESC)
			break; 
	}
	return 0;
}

