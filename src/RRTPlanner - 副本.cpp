#include "RRTPlanner.h"
#include <QDebug> // 用于打印调试信息

RRTPlanner::RRTPlanner() {
}

void RRTPlanner::addObstacle(const SphereObstacle& obs) {
    m_obstacles.push_back(obs);
}

// 碰撞检测核心逻辑
bool RRTPlanner::checkCollision(const cv::Point3f& p1, const cv::Point3f& p2, float threshold) {
    // 遍历所有障碍物，只要撞到一个就算撞
    for (const auto& obs : m_obstacles) {
        float safeRadius = obs.radius + threshold; // 障碍物半径 + 安全余量

        // 1. 向量计算
        cv::Point3f d = p2 - p1; // 线段向量 AB
        cv::Point3f f = p1 - obs.center; // 向量 球心->起点 (这里反过来算比较方便: f = p1 - C)
        
        // 我们用一元二次方程求解: |P(t) - C|^2 = R^2
        // P(t) = p1 + t*d, 其中 0 <= t <= 1
        // 展开后变成: a*t^2 + 2*b*t + c = 0
        
        float a = d.dot(d);
        float b = 2.0f * f.dot(d);
        float c = f.dot(f) - safeRadius * safeRadius;

        // 如果 a=0，说明 p1 和 p2 重合，直接判断点是否在球内
        if (std::abs(a) < 1e-6) {
            if (c < 0) return true; // c < 0 意味着 distance < radius
            continue;
        }

        float discriminant = b*b - 4*a*c; // 判别式 delta

        // 情况 A: delta < 0，方程无解 -> 直线和球不相交 -> 安全
        if (discriminant < 0) {
            continue; 
        }

        // 情况 B: 直线和球相交，但我们要看交点是不是在“线段”上
        // 求两个解 t1, t2
        discriminant = std::sqrt(discriminant);
        float t1 = (-b - discriminant) / (2*a);
        float t2 = (-b + discriminant) / (2*a);

        // 如果区间 [t1, t2] 和 [0, 1] 有重叠，说明线段穿过了球体
        // 因为 t1 肯定 < t2，所以只要满足以下条件就有重叠：
        if (t1 <= 1.0f && t2 >= 0.0f) {
            // 实锤了：碰撞！
            // qDebug() << "💥 检测到碰撞! 障碍物中心:" << obs.center.x << obs.center.y << obs.center.z;
            return true;
        }
    }

    // 遍历完都没事，那就是安全的
    return false;
}