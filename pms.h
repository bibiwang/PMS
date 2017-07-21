#pragma once
#ifndef PMS_HEADER
#define PMS_HEADER

#include <iostream>
#include <opencv2/opencv.hpp>
#include <time.h>
#include <vector>

#define FILENAME "sunny1.mp4"
#define ESC 27
#define DELAYMILL 20
#define PI 3.14159265
#define DEG2RAD PI/180
#define K 2

#define LOWH 0
#define HIGHH 70
#define LOWS 21 
#define HIGHS 255
#define LOWV 31
#define HIGHV 255

#define CARCASCADENAME "C:\\opencv\\build\\etc\\haarcascades\\haarcascade_cars.xml"

using namespace std;
using namespace cv;

typedef struct HoughLineInfo {
	vector<unsigned int> iAngle;
	vector<double> dRho;
}HoughLineInfo;

//DO NOT USE FUNCTION
Mat houghLine(Mat srcFrame) {

	//Exception handling (Image is Empty)
	if (srcFrame.empty() == true) {
		cout << "srcFrame is Empty" << endl;
	}

	Mat dstFrame;
	srcFrame.copyTo(dstFrame);
	// 1.  Edge detection, e.g. using the Canny edge detector.
	Mat edgeFrame;
	GaussianBlur(srcFrame, srcFrame, Size(9, 9), 1.0);
	Canny(srcFrame, edgeFrame, 100, 150);


	//2. Mapping of edge points to the Hough space and storage in an accumulator

	const unsigned int kAngleSize = 180;
	const unsigned int kDistMax = sqrt((edgeFrame.cols*edgeFrame.cols) + (edgeFrame.rows*edgeFrame.rows)) + 1;
	const unsigned int kDistanceSize = kDistMax * 2;
	const unsigned int kThresHoldForLine = 100;

	unsigned int* pNvote = new unsigned int[kAngleSize*kDistanceSize];
	memset(pNvote, 0, (kAngleSize*kDistanceSize) * sizeof(unsigned int));

	double lukCos[180];
	double lukSin[180];
	unsigned int lukAngleIdx[180];

	for (int i = 0; i < kAngleSize; i++)
	{
		double angle = (double)i*DEG2RAD;

		lukCos[i] = cos(angle);
		lukSin[i] = sin(angle);
		lukAngleIdx[i] = i*kDistanceSize;
	}

	unsigned char* ucMatEdgeData = edgeFrame.data;
	for (int y = 0; y < edgeFrame.rows; y++)
	{
		for (int x = 0; x < edgeFrame.cols; x++)
		{
			if (*ucMatEdgeData++ == 0)continue;

			for (unsigned int j = 0; j < kAngleSize; j++)
			{
				double rho = lukCos[j] * x + lukSin[j] * y;
				rho += (double)kDistMax;

				pNvote[lukAngleIdx[j] + (int)(rho + 0.5)]++;
			}
		}
	}

	//3. Interpretation of the accumulator to yield lines of infinite length. 
	//The interpretation is done by thresholding and possibly other constraints.

	Point ptA;
	Point ptB;

	HoughLineInfo lineVec;

	vector<Vec2f> linesVec;

	for (int i = 0; i < 180; i++)
		for (int j = 0; j < kDistanceSize; j++)
		{
			int nVote = pNvote[lukAngleIdx[i] + j];
			if (nVote >= kThresHoldForLine)
			{
				bool isTrueLine = true;

				for (int dAngle = -1; dAngle <= 1 && isTrueLine; dAngle++)
				{
					if (i + dAngle < 0)continue;
					if (i + dAngle >= kAngleSize)break;

					for (int dRho = -1; dRho <= 1 && isTrueLine; dRho++)
					{
						if (j + dRho < 0)continue;
						if (j + dRho >= kDistanceSize)break;

						if (pNvote[lukAngleIdx[i + dAngle] + (j + dRho)] > nVote)isTrueLine = false;
					}
				}

				if (isTrueLine == false) continue;

				lineVec.iAngle.push_back(i);
				lineVec.dRho.push_back(j - (int)kDistMax);
			}
		}

	int nLineVecSize = lineVec.dRho.size();
	for (int i = 0; i < nLineVecSize; i++)
	{
		int angle = lineVec.iAngle[i];

		double cX = lukCos[angle];
		double cY = lukSin[angle];

		double rho = lineVec.dRho[i];

		double x0 = cX*rho;
		double y0 = cY*rho;

		ptA.x = cvRound(x0 + 1000. * (-cY));
		ptA.y = cvRound(y0 + 1000. * (cX));
		ptB.x = cvRound(x0 - 1000. * (-cY));
		ptB.y = cvRound(y0 - 1000. * (cX));

		line(dstFrame, ptA, ptB, Scalar(0, 255, 0), 1);
	}

	delete[] pNvote;

	return dstFrame;
}

Mat kmeansClustering(Mat srcFrame) {
	
	//배열에 8bit uchar type으로 저장되어 있음.
	Mat colorTable(K, 1, CV_8UC3);
	Vec3b color;


	//destination 이미지 만들기
	Mat dstFrame(srcFrame.size(), srcFrame.type());

	//SIZE 추출
	cout << "SIZE : " << srcFrame.size() << endl;

	int attempts = 1;
	int flags = KMEANS_RANDOM_CENTERS;
	TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 1.0);

	Mat samples = srcFrame.reshape(3, srcFrame.rows*srcFrame.cols);
	cout << samples.size() << endl;

	samples.convertTo(samples, CV_32FC3);

	Mat labels;
	srcFrame.convertTo(srcFrame, CV_32FC3);

	kmeans(samples, K, labels, criteria, attempts, flags);

	//output size
	cout << labels.size() << endl;

	// 시간측정
	clock_t begin, end;

	begin = clock();

	for (int y = 0, n = 0; y < srcFrame.rows; y++) {
		for (int x = 0; x < srcFrame.cols; x++, n++) {
			int idx = labels.at<int>(n);
			//idx로 추출함 (idx = 정수배열)
			color = colorTable.at<Vec3b>(idx);
			dstFrame.at<Vec3b>(y, x) = color;
		}
	}

	cout << srcFrame.size() << endl;
	end = clock();

	cout << "K-means 수행시간 : " << (float)(end - begin) / CLOCKS_PER_SEC << endl;

	return dstFrame;
}

Mat preMasking(Mat srcFrame) {
	Mat dstFrame;
	
	//Specific HSV Removal
	Mat hsvFrame, binaryFrame;

	//1. convert to HSV
	cvtColor(srcFrame, hsvFrame, COLOR_BGR2HSV);

	//2. hsv frame to binary frame
	inRange(hsvFrame, Scalar(LOWH, LOWS, LOWV), Scalar(HIGHH, HIGHS, HIGHV), binaryFrame);


	//3. erode and dilate to binary frame
	erode(binaryFrame, binaryFrame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(binaryFrame, binaryFrame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	dilate(binaryFrame, binaryFrame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(binaryFrame, binaryFrame, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//4. not bitwise
	bitwise_not(binaryFrame, binaryFrame);

	return binaryFrame;
}

Mat preprocessing(Mat srcFrame){
	Mat dstFrame, mask;

	//1. masking 
	mask = preMasking(srcFrame);
	srcFrame.copyTo(dstFrame, mask);

	return dstFrame;
}

Mat drawCircle(Mat srcFrame) {
	//장애인 주차장 점
	circle(srcFrame, Point(345, 300), 3, Scalar(0, 0, 255), 1);
	circle(srcFrame, Point(397, 303), 3, Scalar(0, 0, 255), 1);
	circle(srcFrame, Point(402, 355), 3, Scalar(0, 0, 255), 1);
	circle(srcFrame, Point(343, 347), 3, Scalar(0, 0, 255), 1);

	//왼쪽 첫번째 점
	circle(srcFrame, Point(158, 450), 3, Scalar(0, 0, 225), 1);
	//왼쪽 두번째 점
	circle(srcFrame, Point(171, 420), 3, Scalar(0, 0, 225), 1);
	//왼쪽 세번째 점
	circle(srcFrame, Point(189, 382), 3, Scalar(0, 0, 225), 1);
	//왼쪽 네번째 점
	circle(srcFrame, Point(204, 352), 3, Scalar(0, 0, 225), 1);
	//왼쪽 다섯번째 점
	circle(srcFrame, Point(215, 326), 3, Scalar(0, 0, 225), 1);
	//왼쪽 여섯번째 점
	circle(srcFrame, Point(226, 304), 3, Scalar(0, 0, 225), 1);
	//왼쪽 일곱번째 점
	circle(srcFrame, Point(237, 283), 3, Scalar(0, 0, 225), 1);
	//왼쪽 여덟번째 점
	circle(srcFrame, Point(245, 266), 3, Scalar(0, 0, 225), 1);
	//왼쪽 아홉번째 점
	circle(srcFrame, Point(252, 251), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열번째 점
	circle(srcFrame, Point(257, 237), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열한번째 점
	circle(srcFrame, Point(264, 225), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열두번째 점
	circle(srcFrame, Point(269, 214), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열세번째 점
	circle(srcFrame, Point(272, 204), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열네번째 점
	circle(srcFrame, Point(278, 194), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열다섯번째 점
	circle(srcFrame, Point(281, 186), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열여섯번째 점
	circle(srcFrame, Point(285, 177), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열일곱번째 점
	circle(srcFrame, Point(287, 171), 3, Scalar(0, 0, 225), 1);
	//왼쪽 열여덟번째 점
	circle(srcFrame, Point(291, 165), 3, Scalar(0, 0, 225), 1);

	//장애인주차장 오른쪽 첫번째 주차 점
	circle(srcFrame, Point(449, 359), 3, Scalar(0, 0, 225), 1); // 아래 점
	circle(srcFrame, Point(437, 305), 3, Scalar(0, 0, 225), 1); //위 점

															 //장애인주차장 오른쪽 두번째 주차 점
	circle(srcFrame, Point(495, 363), 3, Scalar(0, 0, 225), 1); // 아래 점
	circle(srcFrame, Point(477, 307), 3, Scalar(0, 0, 225), 1); //위 점

															 //장애인주차장 오른쪽 세번째 주차 점
	circle(srcFrame, Point(542, 367), 3, Scalar(0, 0, 225), 1); // 아래 점
	circle(srcFrame, Point(518, 309), 3, Scalar(0, 0, 225), 1); //위 점

															 //장애인주차장 오른쪽 네번째 주차 점
	circle(srcFrame, Point(590, 370), 3, Scalar(0, 0, 225), 1); // 아래 점
	circle(srcFrame, Point(563, 311), 3, Scalar(0, 0, 225), 1); //위 점

															 //장애인주차장 오른쪽 다섯번째 주차 점
	circle(srcFrame, Point(636, 374), 3, Scalar(0, 0, 225), 1); // 아래 점
	circle(srcFrame, Point(605, 314), 3, Scalar(0, 0, 225), 1); //위 점

	return srcFrame;
}

//Mat carHaarCascadeFun(Mat srcFrame) {
//	//Mat dstFrame;
//
//	//srcFrame.copyTo(dstFrame);
//
//	//// Load Face cascade (.xml file)
//	//CascadeClassifier carCascade;
//	//if(!carCascade.load(CARCASCADENAME))
//	//	cout << "Error loading car cascade\n" << endl;
//	//
//	//// Detect cars
//	//vector<Rect> cars;
//	//carCascade.detectMultiScale(srcFrame, cars, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
//
//	//// Draw circles on the detected faces
//	///*for (int i = 0; i < cars.size(); i++){
//	//	Point center(cars[i].x + cars[i].width*0.5, cars[i].y + cars[i].height*0.5);
//	//	ellipse(dstFrame, center, Size(cars[i].width*0.5, cars[i].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
//	//}*/
//
//	//return dstFrame;
//
//}

void mouseClickFun(int event, int x, int y, int flags, void* userdata){

	if (event == EVENT_LBUTTONDOWN){
		cout << "좌표 = (" << x << ", " << y << ")" << endl;
	}
}

#endif