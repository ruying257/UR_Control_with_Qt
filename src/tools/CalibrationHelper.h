#ifndef CALIBRATIONHELPER_H
#define CALIBRATIONHELPER_H

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>
#include <iostream>

class CalibrationHelper
{
public:
    /**
     * @brief 手眼标定核心算法 (Eye-to-Hand, 中继法)
     * 通过腕部相机中继，计算 全局相机 -> 基座 的变换矩阵
     * * @param pose_base_flange  机械臂法兰位姿 (x,y,z, rx,ry,rz) [N组]
     * @param T_flange_wrist    已知的手眼标定矩阵 (法兰->腕部相机)
     * @param wrist_vecs        腕部相机看到的 ArUco (rvec, tvec) [N组]
     * @param global_vecs       全局相机看到的 ArUco (rvec, tvec) [N组]
     * @return cv::Mat          计算出的全局相机变换矩阵 (T_base_global)
     */
    static cv::Mat calibrateGlobalCam(
        const std::vector<std::vector<double>>& poses_base_flange,
        const cv::Mat& T_flange_wrist,
        const std::vector<std::pair<cv::Vec3d, cv::Vec3d>>& wrist_vecs,
        const std::vector<std::pair<cv::Vec3d, cv::Vec3d>>& global_vecs
    );

    // --- 辅助函数 ---
    static cv::Mat poseToMatrix(const std::vector<double>& pose);
    static cv::Mat vectorsToMatrix(const cv::Vec3d& rvec, const cv::Vec3d& tvec);
    static void saveMatrix(const std::string& filename, const cv::Mat& mat);
    static cv::Mat loadMatrix(const std::string& filename);
    
    // ArUco 检测辅助
    static bool detectArUco(const cv::Mat& image, cv::Vec3d& rvec, cv::Vec3d& tvec);
};

#endif // CALIBRATIONHELPER_H