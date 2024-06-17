#pragma once
#include "WindowCapture.h"

class OverlayWindow
{
	public:
	    OverlayWindow(HWND target_hwnd);
	    void show() const;
	    void hide() const;
	    void drawBoundingBoxes(const std::vector<cv::Rect>& boxes, const std::vector<int>& classIds,
			const std::vector<std::string>& classes, const std::unordered_map<std::string, cv::Scalar>& classColors) const;
	
	private:
	    HWND overlay_hwnd;
	    HWND target_hwnd;
	    int width, height;
	
	    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

