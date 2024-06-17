#include <iostream>
#include <Windows.h>
#include <dwmapi.h>
#include <thread>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <dwmapi.h>
#include <fstream>
#include "BehaviourTree.h"
#include "ImageProcessor.h"
#include "OverlayWindow.h"
#include "WindowCapture.h"

volatile bool stopProgram = false;
float g_second(1000);
int g_Width{ 1920 }, g_Height{ 1080 };

// Function to send keyboard input
void sendInput(WORD keyCode, bool keyDown)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode;
    input.ki.dwFlags = keyDown ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Function to simulate a left mouse click
void leftClick(int posX = 0, int posY = 0)
{
    SetCursorPos(posX, posY);

    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    Sleep(g_second / 5.f);

    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void fineMouseMove(int targetX)
{
    // Calculate direction of swipe
    int deltaX = targetX - g_Width/2;

    // Increase the swipe speed for quicker adjustment
    int swipeSpeed = 20;

    // Normalize the direction vector
    double length = std::abs(deltaX);
    if (length != 0)
    {
        deltaX = (int)(deltaX * swipeSpeed / length);
    }

    // Perform the swipe
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = deltaX;
    input.mi.dy = 0; // No change in Y position
    SendInput(1, &input, sizeof(INPUT));
}

// Function to navigate the main menu
void MainMenuNav()
{
    leftClick(0, 0);
    Sleep(g_second * 3.f);
    leftClick(g_Width / 2, 425);
    Sleep(g_second * 2.f);
    leftClick(g_Width / 2, 504);
    Sleep(g_second * 2.f);
    leftClick(g_Width / 2, 475);
    Sleep(g_second * 2.f);
    leftClick(g_Width / 2, 445);
    Sleep(g_second * 2.f);
}

void processEnemyTargeting(const int& enemyCenterX,const int& centerX,const int& thresholdDistance, bool& shootAllowed)
{
    if (std::abs(enemyCenterX - centerX) > thresholdDistance)
    {
        shootAllowed = false;
    }

    if (shootAllowed)
    {
        leftClick(g_Width / 2, g_Height / 2);
    }
    else
    {
        fineMouseMove(enemyCenterX);
    }
}

void processDoorInteraction(const int& doorCenterX, const int& centerX, const int& thresholdDistance)
{
    bool openDoorAllowed = true;

    if (std::abs(doorCenterX - centerX) > thresholdDistance * 2)
    {
        openDoorAllowed = false;
    }

    if (openDoorAllowed)
    {
        sendInput(VK_SPACE, true);
        Sleep(g_second / 0.5f);
        sendInput(VK_SPACE, false);
        sendInput('W', true);
        Sleep(g_second / 2.f);
        sendInput('W', false);
    }
    else
    {
        fineMouseMove(doorCenterX);
    }
}

void navigateToDoor(const int& doorCenterX,const int& centerX, int thresholdDistance)
{
    if (std::abs(doorCenterX - centerX) > thresholdDistance)
    {
        fineMouseMove(doorCenterX);
    }
    else
    {
        sendInput('W', true);
        Sleep(100);
        sendInput('W', false);
    }
}

void AIThread(WindowCapture& wincap, ImageProcessor& improc, OverlayWindow& overlay, std::vector<std::string>& classes)
{
    const int thresholdDistance = 15;
    int centerX = wincap.getWidth() / 2;

    while (true)
    {
        cv::Mat ss = wincap.getScreenShot();
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        improc.processImage(ss, classIds, confidences, boxes);

        overlay.drawBoundingBoxes(boxes, classIds, classes, improc.classColors);

        bool aimAtEnemy = false;
        bool aimAtDoor = false;
        int enemyCenterX = 0;
        int doorCenterX = 0;
        for (size_t i = 0; i < classIds.size(); ++i)
        {
            sendInput('W', false);
            sendInput('D', false);

            if ((classes[classIds[i]] == "human" || classes[classIds[i]] == "alien") && confidences[i] > 0.8)
            {
                enemyCenterX = boxes[i].x + boxes[i].width / 2;
                aimAtEnemy = true;
                break;
            }
        	if(classes[classIds[i]] == "door" && confidences[i] > 0.8)
            {
                doorCenterX = boxes[i].x + boxes[i].width / 2;
                aimAtDoor = true;
                break;
            }
        }

        bool shootAllowed = true;

        if (aimAtEnemy)
        {
            processEnemyTargeting(enemyCenterX, centerX, thresholdDistance, shootAllowed);
        }
        else if (aimAtDoor)
        {
            navigateToDoor(doorCenterX, centerX, thresholdDistance);
            processDoorInteraction(doorCenterX, centerX, thresholdDistance);
        }
        else
        {
            sendInput('W', true);
            sendInput('D', true);
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main()
{
    const std::string window_name = "DOOM";
    const std::string cfg_file_name = "Resources/YOLO/yolov4-tiny-custom.cfg";
    const std::string weights_file_name = "Resources/YOLO/yolov4-tiny-custom_last.weights";
    const std::string classes_file_name = "Resources/YOLO/obj.names";

    std::vector<std::string> classes;
    std::ifstream ifs(classes_file_name.c_str());
    std::string line;
    while (std::getline(ifs, line))
        classes.push_back(line);

    WindowCapture wincap(window_name);
    ImageProcessor improc(cv::Size(wincap.getWidth(), wincap.getHeight()),
        cfg_file_name, weights_file_name, classes_file_name);
    OverlayWindow overlay(wincap.getWindow());

    SetForegroundWindow(wincap.getWindow());
    MainMenuNav();
    overlay.show();
    SetForegroundWindow(wincap.getWindow());

    std::thread ai_thread(AIThread, std::ref(wincap),
        std::ref(improc), std::ref(overlay), std::ref(classes));

    ai_thread.join();

    overlay.hide();
    
    return 0;
}