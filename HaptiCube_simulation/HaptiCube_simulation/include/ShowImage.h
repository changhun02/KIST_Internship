#pragma once
#ifndef SHOWIMAGE_H
#define SHOWIMAGE_H

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class ShowImage {
public :
	static void show(const char* WinName, const Mat& img, int delay = 1);
	static char getkey();

protected :
	static char key;
};

char ShowImage::key = -1;

void ShowImage::show(const char* WinName, const Mat& img, int delay) {
	imshow(WinName, img);
	char ch = waitKey(delay);
	if (ch != -1) key = ch;
}

char ShowImage::getkey() {
	char rect = key;
	key = -1;
	return rect;
}
#endif