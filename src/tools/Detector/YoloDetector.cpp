#include "YoloDetector.h"
#include <QDebug> // 使用 Qt 的日志输出，跨平台方便

YoloDetector::YoloDetector() {
    // 根据模型类别修改
    m_classNames = { "nut" }; 
}

bool YoloDetector::loadModel(const std::string& onnxPath) {
    try {
        qDebug() << "🔄 正在加载 YOLO 模型:" << QString::fromStdString(onnxPath);
        m_net = cv::dnn::readNetFromONNX(onnxPath);

        // ==========================================
        // ⚡ 跨平台配置策略 (Windows & Jetson)
        // ==========================================
        // 为了确保最快跑通，我们强制使用 CPU 后端。
        // Jetson 上虽然有 GPU，但配置 CUDA 版 OpenCV 非常耗时，CPU 跑小模型足够了。
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        qDebug() << "✅ 模型加载成功! Backend: CPU";
        return true;
    } catch (const cv::Exception& e) {
        qDebug() << "❌ 模型加载异常:" << e.what();
        return false;
    }
}

cv::Point2f YoloDetector::detect(const cv::Mat& img, cv::Mat& debugImg) {
    // 1. 准备 Debug 图像
    if (img.data != debugImg.data) {
        img.copyTo(debugImg);
    }
    if (img.empty()) return cv::Point2f(-1, -1);
    if (m_net.empty()) {
        qDebug() << "⚠️ 警告: 模型未加载，无法检测";
        return cv::Point2f(-1, -1);
    }

    // 2. 图像预处理 (Blob)
    // YOLO 要求归一化 0~1 (scale=1/255)，SwapRB=true (BGR->RGB)，不裁剪
    cv::Mat blob;
    // 从图像创建 blob，保持比例，填充到 INPUT_W x INPUT_H
    cv::dnn::blobFromImage(img, blob, 1.0/255.0, cv::Size(INPUT_W, INPUT_H), cv::Scalar(), true, false);

    // 3. 推理 (Inference)
    m_net.setInput(blob);
    
    // 获取输出层
    std::vector<cv::Mat> outputs;
    m_net.forward(outputs, m_net.getUnconnectedOutLayersNames());

    // ==========================================================
    // 🧩 核心难点：YOLOv12 输出解析 (OpenCV 4.6 兼容写法)
    // ==========================================================
    // YOLOv8 输出维度通常是 [1, 4+Classes, 8400]
    // C++ OpenCV 处理行优先数据方便，所以我们需要把矩阵转置 (Transpose)
    // 变成 [1, 8400, 4+Classes]
    
    // 假设 outputs[0] 是主要输出
    cv::Mat output0 = outputs[0];
    
    // 检查维度，如果不匹配可能需要调整 (这里适配标准的 YOLOv12 导出)
    int dimensions = output0.size[1]; // 4 + classes
    int rows = output0.size[2];       // 8400 anchors
    
    // 如果维度是 [1, dimensions, rows]，我们需要转置
    if (dimensions > rows) {
        // 检查是否是 [1, dimensions, rows]
        if (output0.size[0] == 1 && output0.size[2] == rows) {
            // 说明是 [1, dimensions, rows]，需要转置
            rows = output0.size[1];
            dimensions = output0.size[2];
        } else {
            // 其他情况，保持不变
            rows = output0.size[2];
            dimensions = output0.size[1];
        }
    } else {
        // 常见情况：需要转置
        // 重新构造一个 2D 矩阵 [dimensions, rows]
        output0 = cv::Mat(dimensions, rows, CV_32F, output0.ptr<float>());
        // 转置为 [rows, dimensions] -> [8400, 5]
        cv::transpose(output0, output0);
    }

    // 4. 解析数据
    float* data = (float*)output0.data;
    float x_factor = (float)img.cols / INPUT_W;
    float y_factor = (float)img.rows / INPUT_H;
    
    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i) {
        // 每一行数据：[cx, cy, w, h, score]
        // 前4个是坐标，代表框的中心和宽高；
        // 后面的 score 是类别的置信度（只有一个类）
        float conf = data[4]; // 跳过前4个坐标，直接取置信度

        if (conf > CONFIDENCE_THRESHOLD) {
            float cx = data[0];
            float cy = data[1];
            float w = data[2];
            float h = data[3];

            // 还原回原图尺寸
            int left = int((cx - 0.5 * w) * x_factor);
            int top = int((cy - 0.5 * h) * y_factor);
            int width = int(w * x_factor);
            int height = int(h * y_factor);

            boxes.push_back(cv::Rect(left, top, width, height));
            confidences.push_back(conf);
            class_ids.push_back(0); // 只有一个类，固定为0
        }
        
        // 指针移动到下一行 (dimensions 是步长)
        data += dimensions;
    }

    // 5. NMS (非极大值抑制) - 去除重叠框
    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);

    // 6. 选取最佳结果
    cv::Point2f bestCenter(-1, -1);
    float bestConf = -1.0;

    for (int idx : nms_result) {
        cv::Rect box = boxes[idx];
        float conf = confidences[idx];
        int classId = class_ids[idx];

        // 绘制结果
        cv::rectangle(debugImg, box, cv::Scalar(0, 255, 0), 2);
        
        std::string label = m_classNames.size() > classId ? 
                            m_classNames[classId] : std::to_string(classId);
        label += " " + std::to_string(conf).substr(0, 4);
        
        cv::putText(debugImg, label, cv::Point(box.x, box.y - 5), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

        // 策略：返回置信度最高的那个
        if (conf > bestConf) {
            bestConf = conf;
            bestCenter = cv::Point2f(box.x + box.width / 2.0f, 
                                     box.y + box.height / 2.0f);
        }
    }
    return bestCenter;
}