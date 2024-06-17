#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <unordered_map>

class ImageProcessor
{
	public:
		ImageProcessor(cv::Size img_size, const std::string& cfg_file, const std::string& weights_file, const std::string& name_file);
		void processImage(cv::Mat& img, std::vector<int>& classIds, std::vector<float>& confidences, std::vector<cv::Rect>& boxes);

		std::unordered_map<std::string, cv::Scalar> classColors;

	private:
		int W, H;
		cv::dnn::Net net;
		std::vector<std::string> classes;
		std::vector<std::string> getOutputsNames() const;
		void postProcess(const std::vector<cv::Mat>& outs, std::vector<int>& classIds, std::vector<float>& confidences, std::vector<cv::Rect>& boxes) const;
};
