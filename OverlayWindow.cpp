#include "OverlayWindow.h"

OverlayWindow::OverlayWindow(HWND target_hwnd) : target_hwnd(target_hwnd)
{
    RECT rect;
    GetWindowRect(target_hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = windowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"OverlayWindow";
    wcex.hIconSm = NULL;

    RegisterClassEx(&wcex);

    overlay_hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        L"OverlayWindow",
        L"Overlay",
        WS_POPUP,
        rect.left, rect.top, width, height,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    SetLayeredWindowAttributes(overlay_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
}

void OverlayWindow::show() const
{
    ShowWindow(overlay_hwnd, SW_SHOW);
}

void OverlayWindow::hide() const
{
    ShowWindow(overlay_hwnd, SW_HIDE);
}

void OverlayWindow::drawBoundingBoxes(const std::vector<cv::Rect>& boxes, const std::vector<int>& classIds, const std::vector<std::string>& classes, const std::unordered_map<std::string, cv::Scalar>& classColors) const
{
    HDC hdc = GetDC(overlay_hwnd);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbm = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(hdcMem, hbm);

    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(hdcMem, hBrush);

    for (size_t i = 0; i < boxes.size(); ++i)
    {
        cv::Rect box = boxes[i];
        int classId = classIds[i];
        std::string className = classes[classId];
        cv::Scalar color = classColors.count(className) ? classColors.at(className) : cv::Scalar(255, 255, 255); // Default to white if not found

        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(color[2], color[1], color[0]));
        SelectObject(hdcMem, hPen);
        Rectangle(hdcMem, box.x, box.y, box.x + box.width, box.y + box.height);
        DeleteObject(hPen);
    }

    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

    DeleteObject(hbm);
    DeleteDC(hdcMem);
    ReleaseDC(overlay_hwnd, hdc);
}

LRESULT CALLBACK OverlayWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}