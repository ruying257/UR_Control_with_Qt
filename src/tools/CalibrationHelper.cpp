#include "CalibrationHelper.h"

// 假设 ArUco 字典和大小 (请根据实际情况修改)
// 边长 0.05 米 (5cm)
const float MARKER_LENGTH = 0.05f; 
const auto DICTIONARY = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

cv::Mat CalibrationHelper::calibrateGlobalCam(
    const std::vector<std::vector<double>>& poses_base_flange,
    const cv::Mat& T_flange_wrist,
    const std::vector<std::pair<cv::Vec3d, cv::Vec3d>>& wrist_vecs,
    const std::vector<std::pair<cv::Vec3d, cv::Vec3d>>& global_vecs
) {
    if (poses_base_flange.empty()) return cv::Mat();

    cv::Mat T_accum = cv::Mat::zeros(4, 4, CV_64F);
    int valid_count = 0;

    // 我们采集了 N 组数据，计算 N 个 T_base_global，然后取平均
    // 更好的做法是求旋转矩阵的平均（四元数插值），这里简化为矩阵元素平均，
    // 对于工业现场的静态标定，只要数据波动不大，均值法足够用。
    
    for (size_t i = 0; i < poses_base_flange.size(); ++i) {
        // 1. 获取各环节矩阵
        cv::Mat T_base_flange = poseToMatrix(poses_base_flange[i]);
        cv::Mat T_wrist_aruco = vectorsToMatrix(wrist_vecs[i].first, wrist_vecs[i].second);
        cv::Mat T_global_aruco = vectorsToMatrix(global_vecs[i].first, global_vecs[i].second);

        // 2. 链路计算
        // T_base_aruco = T_base_flange * T_flange_wrist * T_wrist_aruco
        cv::Mat T_base_aruco = T_base_flange * T_flange_wrist * T_wrist_aruco;

        // T_base_global = T_base_aruco * (T_global_aruco)^-1
        cv::Mat T_base_global = T_base_aruco * T_global_aruco.inv();

        T_accum += T_base_global;
        valid_count++;
    }

    if (valid_count == 0) return cv::Mat();
    return T_accum / valid_count;
}

bool CalibrationHelper::detectArUco(const cv::Mat& image, cv::Vec3d& rvec, cv::Vec3d& tvec) {
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    
    cv::aruco::detectMarkers(image, DICTIONARY, corners, ids);
    
    if (!ids.empty()) {
        // 假设你要找 ID=0 的码
        for(size_t i=0; i<ids.size(); ++i) {
            if(ids[i] == 0) {
                // 相机内参 (这里简化为单位矩阵，实际一定要填入 calibrateCamera 的结果！)
                cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F); 
                // 粗略估计内参: fx=fy=width, cx=w/2, cy=h/2
                cameraMatrix.at<double>(0,0) = image.cols; 
                cameraMatrix.at<double>(1,1) = image.cols;
                cameraMatrix.at<double>(0,2) = image.cols / 2;
                cameraMatrix.at<double>(1,2) = image.rows / 2;
                
                cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);

                std::vector<cv::Vec3d> rvecs, tvecs;
                cv::aruco::estimatePoseSingleMarkers(corners, MARKER_LENGTH, cameraMatrix, distCoeffs, rvecs, tvecs);
                
                rvec = rvecs[i];
                tvec = tvecs[i];
                return true;
            }
        }
    }
    return false;
}

cv::Mat CalibrationHelper::poseToMatrix(const std::vector<double>& pose) {
    cv::Vec3d tvec(pose[0], pose[1], pose[2]);
    cv::Vec3d rvec(pose[3], pose[4], pose[5]);
    return vectorsToMatrix(rvec, tvec);
}

cv::Mat CalibrationHelper::vectorsToMatrix(const cv::Vec3d& rvec, const cv::Vec3d& tvec) {
    cv::Mat R;
    cv::Rodrigues(rvec, R);
    cv::Mat T = cv::Mat::eye(4, 4, CV_64F);
    R.copyTo(T(cv::Rect(0, 0, 3, 3)));
    T.at<double>(0, 3) = tvec[0];
    T.at<double>(1, 3) = tvec[1];
    T.at<double>(2, 3) = tvec[2];
    return T;
}

void CalibrationHelper::saveMatrix(const std::string& filename, const cv::Mat& mat) {
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);
    fs << "T_base_global" << mat;
    fs.release();
    std::cout << "💾 结果已保存至: " << filename << std::endl;
}

cv::Mat CalibrationHelper::loadMatrix(const std::string& filename) {
    cv::Mat mat;
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    fs["T_base_global"] >> mat;
    fs.release();
    return mat;
}