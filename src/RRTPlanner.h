#ifndef RRTPLANNER_H
#define RRTPLANNER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <random> // 用于生成随机数

// 定义障碍物 (保持不变)
struct SphereObstacle {
    cv::Point3f center;
    float radius;
};

// 【新增】树的节点
struct Node {
    cv::Point3f pos; // 当前点的位置
    int parentId;    // 父节点在数组中的索引 (-1表示根节点)
};

class RRTPlanner
{
public:
    RRTPlanner();

    void addObstacle(const SphereObstacle& obs);
    bool checkCollision(const cv::Point3f& p1, const cv::Point3f& p2, float threshold = 0.05);

    /**
     * @brief 核心函数：规划路径
     * @param start 起点
     * @param goal 终点
     * @return 路径点集合 (从起点到终点)
     */
    std::vector<cv::Point3f> planPath(const cv::Point3f& start, const cv::Point3f& goal);

private:
    std::vector<SphereObstacle> m_obstacles;

    // --- RRT 辅助参数 ---
    float m_stepSize = 0.05;   // 步长: 每次生长 5cm (太大会穿墙，太小算得慢)
    float m_maxIter = 5000;    // 最大尝试次数: 防止死循环
    float m_goalBias = 0.1;    // 目标偏向概率: 10% 的概率直接向终点生长 (加速收敛)

    // 工作空间边界 (单位: 米) - 防止随机点撒到无穷远
    float x_min = -0.8, x_max = 0.8;
    float y_min = -0.8, y_max = 0.8;
    float z_min =  0.0, z_max = 1.0;

    // --- 内部辅助函数 ---
    // 1. 生成一个随机点
    cv::Point3f getRandomPoint(const cv::Point3f& goal);
    // 2. 找到树中离随机点最近的节点索引
    int getNearestNodeId(const std::vector<Node>& tree, const cv::Point3f& point);
    // 3. 从 'from' 向 'to' 移动一小步，返回新点
    cv::Point3f step(const cv::Point3f& from, const cv::Point3f& to);
    // 4. 计算两点距离
    float distance(const cv::Point3f& p1, const cv::Point3f& p2);
};

#endif // RRTPLANNER_H
