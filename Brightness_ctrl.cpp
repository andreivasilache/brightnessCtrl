#include "highlevelmonitorconfigurationapi.h"
#include "physicalmonitorenumerationapi.h"
#include "opencv2/opencv.hpp"
#include <winuser.h> // includes windows.h
#include "Ntddvdeo.h"
#include <math.h>


#pragma comment(lib, "Dxva2.lib") // Link Dxva2.lib to executable.

#include <iostream>
using namespace cv;

const std::wstring GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::wstring(); //No error message has been recorded

	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	std::wstring message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

bool setBrightness(int value) {
	 // if (value > 0 && value < 100) {
		typedef struct _DISPLAY_BRIGHTNESS {
			UCHAR ucDisplayPolicy;
			UCHAR ucACBrightness;
			UCHAR ucDCBrightness;
		} DISPLAY_BRIGHTNESS, * PDISPLAY_BRIGHTNESS;

		HANDLE h = CreateFile(L"\\\\.\\LCD",
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0, NULL);
		DWORD IOCTL_VIDEO_SET_DISPLAY_BRIGHTNESS; //
		DISPLAY_BRIGHTNESS _displayBrightness;

		_displayBrightness.ucDisplayPolicy = 100;
		_displayBrightness.ucACBrightness = 0;  
		_displayBrightness.ucDCBrightness = value;

		DWORD nOutBufferSize = sizeof(_displayBrightness);
		DWORD ret = NULL;

		if (h == INVALID_HANDLE_VALUE) {
			//Does not reach here
			std::cout << "no value here!";
			return false;
		}
		return DeviceIoControl(h, IOCTL_VIDEO_SET_DISPLAY_BRIGHTNESS, (DISPLAY_BRIGHTNESS*)&_displayBrightness, nOutBufferSize, NULL, 0, &ret, NULL);
	//}
	// return false;
}



int main(int, char**)
{

	const int MaxBrightnessValue = 120;
	
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	Mat edges;
	namedWindow("edges", 1);
	for (;;)
	{
		try {
			Mat frame;
			cap >> frame; // get a new frame from camera
			cvtColor(frame, edges, COLOR_RGB2RGBA);
			
			int totalPixels = frame.rows * frame.cols;
			int blueAvg = 0, 
				greenAvg = 0, 
				redAvg = 0;
			float luminosityValue = 0;
			
			imshow("edges", edges);

			for (int r = 0; r < frame.rows; ++r) {
				for (int c = 0; c < frame.cols; ++c) {
					blueAvg += frame.at<Vec3b>(r, c)[0];
					greenAvg += frame.at<Vec3b>(r, c)[1];
					redAvg += frame.at<Vec3b>(r, c)[2];
				}
			}

			blueAvg /= totalPixels;
			redAvg /= totalPixels;
			greenAvg /= totalPixels;

			luminosityValue = 0.2126 * redAvg + 0.7152 * greenAvg + 0.0722 * blueAvg;
			if (luminosityValue >= 120) luminosityValue = 120;
			
			const int lumVal = std::ceil((100 * luminosityValue) / MaxBrightnessValue / 10) * 10;
		
				if (setBrightness(lumVal)) {
					std::cout << "\n L:" << lumVal;
				}
	
			if (waitKey(30) >= 0) break;
		}
		catch (const std::exception & e) {
			std::cout << "Doesn't work : " << e.what();
		}
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}