#pragma once
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <string>

class WindowCapture
{
	public:
		WindowCapture(const std::string& window_name);
		HWND getWindow() const { return hwnd; }
		int getWidth() const { return w; }
		int getHeight() const { return h; }
		cv::Mat getScreenShot() const;

	private:
		int w, h;
		HWND hwnd;
};
