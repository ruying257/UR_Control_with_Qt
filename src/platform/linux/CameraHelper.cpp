#include "../CameraHelper.h"

cv::VideoCapture createCamera(int index) {
    cv::VideoCapture cap;
    // Linux 下使用 V4L2
    if (cap.open(index, cv::CAP_V4L2)) {
        // Linux 特有优化顺序：先设格式，再设分辨率
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

        // 可以在这里加 read() 测试是否是坏节点

        qDebug() << "🐧 [Linux] 相机" << index << "初始化成功";
    } else {
        qDebug() << "❌ [Linux] 无法打开相机" << index;
    }
    return cap;
}
