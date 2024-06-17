#include "ImageProcessor.h"
#include <fstream>

ImageProcessor::ImageProcessor(cv::Size img_size, const std::string& cfg_file, const std::string& weights_file, const std::string& name_file)
{
    net = cv::dnn::readNetFromDarknet(cfg_file, weights_file);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    W = img_size.width;
    H = img_size.height;

    std::ifstream ifs(name_file);
    std::string line;
    while (getline(ifs, line))
    {
        classes.push_back(line);
    }

    // Initialize the class colors map
    classColors =
    {
        {"door", cv::Scalar(0, 255, 255)},  // Yellow
        {"human", cv::Scalar(0, 255, 0)},   // Green
        {"alien", cv::Scalar(0, 0, 255)}    // Red
    };
}

void ImageProcessor::processImage(cv::Mat& img, std::vector<int>& classIds,
    std::vector<float>& confidences, std::vector<cv::Rect>& boxes)
{
    cv::Mat blob = cv::dnn::blobFromImage(img, 1.0 / 255.0, cv::Size(416, 416),
        cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    std::vector<cv::Mat> outs;
    net.forward(outs, getOutputsNames());

    postProcess(outs, classIds, confidences, boxes);
}

std::vector<std::string> ImageProcessor::getOutputsNames() const
{
    static std::vector<std::string> names;
    if (names.empty())
    {
        std::vector<int> outLayers = net.getUnconnectedOutLayers();
        std::vector<std::string> layersNames = net.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

void ImageProcessor::postProcess(const std::vector<cv::Mat>& outs, std::vector<int>& classIds,
    std::vector<float>& confidences, std::vector<cv::Rect>& boxes) const
{
    for (size_t i = 0; i < outs.size(); ++i)
    {
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > 0.5)
            {
                int centerX = (int)(data[0] * W);
                int centerY = (int)(data[1] * H);
                int width = (int)(data[2] * W);
                int height = (int)(data[3] * H);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back(static_cast<float>(confidence));
                boxes.emplace_back(cv::Rect(left, top, width, height));
            }
        }
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, 0.5f, 0.4f, indices);
    std::vector<cv::Rect> filtered_boxes;
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        filtered_boxes.push_back(boxes[idx]);
    }
    boxes = filtered_boxes;
}
