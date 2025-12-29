#include "../CameraHelper.h"

cv::VideoCapture createCamera(int index) {
    cv::VideoCapture cap;
    // Windows 下使用 DirectShow
    if (cap.open(index, cv::CAP_DSHOW)) {
        // Windows 通用配置
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

        qDebug() << "🪟 [Windows] 相机" << index << "初始化成功";
    } else {
        qDebug() << "❌ [Windows] 无法打开相机" << index;
    }
    return cap;
}
