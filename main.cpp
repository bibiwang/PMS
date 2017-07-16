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

	Mat frame, houghFrame, kmeansFrame, ppFrame;
	int flag = 0;

	//HSV Trackbar
	namedWindow("찾을 색범위 설정", CV_WINDOW_AUTOSIZE);

	//트랙바에서 사용되는 변수 초기화 
	int LowH = 170;
	int HighH = 179;

	int LowS = 50;
	int HighS = 255;

	int LowV = 0;
	int HighV = 255;

	//트랙바 생성 
	cvCreateTrackbar("LowH", "찾을 색범위 설정", &LowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "찾을 색범위 설정", &HighH, 179);

	cvCreateTrackbar("LowS", "찾을 색범위 설정", &LowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "찾을 색범위 설정", &HighS, 255);

	cvCreateTrackbar("LowV", "찾을 색범위 설정", &LowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "찾을 색범위 설정", &HighV, 255);

	//MAIN LOOP
	while(true){ 
		video >> frame;

		//FPS 
		string fps = to_string((int)video.get(CV_CAP_PROP_FPS));
		putText(frame, "FPS : " + fps, Point(10, 40), 2, 1.2, Scalar(0, 255, 0));

		//houghFrame = houghLine(frame);
		//kmeansFrame = kmeansClustering(frame);
		//ppFrame = preprocessing(frame);
		imshow("PMS", frame);

		//Pushing esc key is terminate
		if (waitKey(DELAYMILL) == ESC)
			break; 
	}
	return 0;
}

