#include "../RRTPlanner.h"
#include <QDebug>

int main() {
    qDebug() << "🚀 正在启动 RRT 算法单元测试...";

    RRTPlanner planner;

    // 1. 设置虚拟环境
    SphereObstacle obs;
    obs.center = cv::Point3f(0.5, 0.0, 0.0);
    obs.radius = 0.1;
    planner.addObstacle(obs);
    qDebug() << "📦 已添加虚拟障碍物: Center(0.5, 0, 0), R=0.1";

    // 2. 运行碰撞测试
    cv::Point3f start(0.0, 0.0, 0.0);
    cv::Point3f end(1.0, 0.0, 0.0);
    
    bool isCollided = planner.checkCollision(start, end);

    if (isCollided) {
        qDebug() << "✅ 测试通过: 正确检测到碰撞!";
    } else {
        qDebug() << "❌ 测试失败: 未检测到碰撞!";
    }

    return 0;
}
