#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include <opencv2/opencv.hpp>
#include <QDebug>

/**
 * @brief 跨平台相机初始化函数
 * @param index 相机索引
 * @return 初始化好的 cv::VideoCapture 对象 (如果失败则返回未打开的对象)
 */
cv::VideoCapture createCamera(int index);

#endif // CAMERAHELPER_H
