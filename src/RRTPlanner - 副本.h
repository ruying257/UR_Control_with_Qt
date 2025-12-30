#ifndef RRTPLANNER_H
#define RRTPLANNER_H

#include <opencv2/opencv.hpp> // 使用 cv::Point3f
#include <vector>
#include <cmath>

// 定义球形障碍物
struct SphereObstacle {
    cv::Point3f center; // 球心 (x, y, z) 单位: 米
    float radius;       // 半径 单位: 米
};

class RRTPlanner
{
public:
    RRTPlanner();

    // 添加一个障碍物到环境里
    void addObstacle(const SphereObstacle& obs);

    // 【核心数学题】检测线段是否与球体碰撞
    // p1: 线段起点
    // p2: 线段终点
    // threshold: 安全距离（膨胀半径），通常设为 0.05 (5cm)，给机械臂留点余量
    // 返回: true 表示撞上了
    bool checkCollision(const cv::Point3f& p1, const cv::Point3f& p2, float threshold = 0.0);

private:
    std::vector<SphereObstacle> m_obstacles; // 存储所有的障碍物
};

#endif // RRTPLANNER_H