#ifndef YOLODETECTOR_H
#define YOLODETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>

struct Detection {
    int class_id;
    float confidence;
    cv::Rect box;
};

class YoloDetector {
public:
    YoloDetector();

    // 初始化模型
    bool loadModel(const std::string& onnxPath);

    /**
     * @brief 执行推理
     * @param img 输入图像
     * @param debugImg 用于画框的图
     * @return 目标中心点 (如果没有检测到，返回 -1,-1)
     */
    cv::Point2f detect(const cv::Mat& img, cv::Mat& debugImg);

private:
    cv::dnn::Net m_net;
    std::vector<std::string> m_classNames;

    // YOLO 参数 (根据模型训练时的尺寸修改，通常是 640)
    const int INPUT_W = 640;
    const int INPUT_H = 640;

    const float SCORE_THRESHOLD = 0.5; // 置信度阈值
    const float NMS_THRESHOLD = 0.4;   // 非极大值抑制阈值
};

#endif // YOLODETECTOR_H
