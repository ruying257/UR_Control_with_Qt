#include <iostream>
#include <vector>
#include <string>
#include "CalibrationHelper.h"
#include "../platform/CameraHelper.h" // 复用之前的相机打开函数

using namespace std;
using namespace cv;

// 已知的腕部相机手眼标定结果 (示例值，请修改!)
Mat getKnownHandEye() {
    Mat T = Mat::eye(4, 4, CV_64F);
    // 假设腕部相机在法兰中心前方 0.1m, Z轴一致
    T.at<double>(2, 3) = 0.1; 
    return T;
}

int main() {
    cout << "========================================" << endl;
    cout << "   🤖 机械臂双目中继标定工具 (CLI Mode)   " << endl;
    cout << "========================================" << endl;

    // 1. 初始化相机
    cout << "📷 正在打开相机..." << endl;
    VideoCapture capWrist = createCamera(0); // 假设 0 是腕部
    VideoCapture capGlobal = createCamera(1); // 假设 1 是全局
    
    if (!capWrist.isOpened() || !capGlobal.isOpened()) {
        cerr << "❌ 错误: 无法打开相机! 请检查连接。" << endl;
        return -1;
    }
    cout << "✅ 相机就绪。" << endl;

    // 数据容器
    vector<vector<double>> poses;
    vector<pair<Vec3d, Vec3d>> wrist_data;
    vector<pair<Vec3d, Vec3d>> global_data;

    // 2. 交互循环
    while (true) {
        cout << "\n----------------------------------------" << endl;
        cout << "当前已采集: " << poses.size() << " 组数据" << endl;
        cout << "请操作:\n [Enter] 采集一组数据\n [c] 计算并保存\n [q] 退出" << endl;
        cout << ">> ";
        
        char cmd = cin.get();
        // 清除输入缓冲区的换行符
        if (cmd != '\n') while(cin.get() != '\n'); 

        if (cmd == 'q') break;
        
        if (cmd == 'c') {
            if (poses.size() < 3) {
                cout << "⚠️ 数据太少，建议至少采集 3 组!" << endl;
                continue;
            }
            cout << "🧮 正在计算..." << endl;
            Mat T = CalibrationHelper::calibrateGlobalCam(poses, getKnownHandEye(), wrist_data, global_data);
            
            if (!T.empty()) {
                cout << "🎉 标定成功! 变换矩阵 T_base_global:" << endl;
                cout << T << endl;
                CalibrationHelper::saveMatrix("calibration_result.yml", T);
            } else {
                cerr << "❌ 计算失败，可能是数据无效。" << endl;
            }
            continue;
        }

        // --- 采集流程 ---
        
        // A. 抓图检测
        Mat frameWrist, frameGlobal;
        capWrist >> frameWrist;
        capGlobal >> frameGlobal;

        Vec3d rWrist, tWrist, rGlobal, tGlobal;
        bool okW = CalibrationHelper::detectArUco(frameWrist, rWrist, tWrist);
        bool okG = CalibrationHelper::detectArUco(frameGlobal, rGlobal, tGlobal);

        if (!okW) cout << "❌ 腕部相机未检测到 ArUco!" << endl;
        if (!okG) cout << "❌ 全局相机未检测到 ArUco!" << endl;

        if (okW && okG) {
            cout << "✅ 视觉检测成功!" << endl;
            
            // B. 输入机械臂坐标
            cout << "请输入示教器上的坐标 (m, rad) [格式: x y z rx ry rz]: ";
            vector<double> p(6);
            cin >> p[0] >> p[1] >> p[2] >> p[3] >> p[4] >> p[5];
            while(cin.get() != '\n'); // 清缓冲

            // 存入容器
            poses.push_back(p);
            wrist_data.push_back({rWrist, tWrist});
            global_data.push_back({rGlobal, tGlobal});
            
            cout << "📦 数据已保存." << endl;
        } else {
            cout << "⚠️ 请调整位置后重试。" << endl;
        }
    }

    return 0;
}