#include "RRTPlanner.h"
#include <QDebug>
#include <iostream>

int main() {
    qDebug() << "🚀 启动 RRT 路径规划测试...";

    RRTPlanner planner;

    // 1. 设置障碍物 (挡在起点和终点中间)
    SphereObstacle obs;
    obs.center = cv::Point3f(0.5, 0.0, 0.0);
    obs.radius = 0.1; // 半径 10cm
    planner.addObstacle(obs);

    qDebug() << "🧱 障碍物已放置:" << obs.center << ", R=" << obs.radius;

    // 2. 设置起点和终点
    cv::Point3f start(0.0, 0.0, 0.0);
    cv::Point3f goal(1.0, 0.0, 0.0);

    // 3. 开始规划
    qDebug() << "🏃 开始计算路径...";
    std::vector<cv::Point3f> path = planner.planPath(start, goal);

    // 4. 打印结果
    if (!path.empty()) {
        qDebug() << "🎉 成功规划出路径! 节点总数:" << path.size();
        std::cout << "Path: [ ";
        for (const auto& p : path) {
            // 简单打印几个关键点，格式: (x, y, z)
            std::cout << "(" << p.x << "," << p.y << "," << p.z << ") -> ";
        }
        std::cout << "GOAL ]" << std::endl;
    } else {
        qDebug() << "❌ 规划失败!";
    }

    return 0;
}
