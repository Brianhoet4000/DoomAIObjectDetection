#include "WindowCapture.h"
#include <stdexcept>

WindowCapture::WindowCapture(const std::string& window_name)
{
    hwnd = FindWindowA(NULL, window_name.c_str());
    if (!hwnd)
    {
        throw std::runtime_error("Window not found: " + window_name);
    }

    RECT window_rect;
    GetWindowRect(hwnd, &window_rect);
    w = window_rect.right - window_rect.left;
    h = window_rect.bottom - window_rect.top;

    int border_pixels = 8;
    int titlebar_pixels = 30;
    w -= (border_pixels * 2);
    h -= titlebar_pixels + border_pixels;
}

cv::Mat WindowCapture::getScreenShot() const
{
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, w, h);
    SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, w, h, hdcWindow, 8, 30, SRCCOPY);

    BITMAPINFOHEADER bitmapInfo;
    bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.biWidth = w;
    bitmapInfo.biHeight = -h;
    bitmapInfo.biPlanes = 1;
    bitmapInfo.biBitCount = 24;
    bitmapInfo.biCompression = BI_RGB;
    bitmapInfo.biSizeImage = 0;
    bitmapInfo.biXPelsPerMeter = 0;
    bitmapInfo.biYPelsPerMeter = 0;
    bitmapInfo.biClrUsed = 0;
    bitmapInfo.biClrImportant = 0;

    cv::Mat img(h, w, CV_8UC3);
    GetDIBits(hdcWindow, hbmScreen, 0, (UINT)h, img.data, (BITMAPINFO*)&bitmapInfo, DIB_RGB_COLORS);

    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    return img;
}