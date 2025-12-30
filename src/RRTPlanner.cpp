#include "RRTPlanner.h"
#include <QDebug>
#include <limits>

RRTPlanner::RRTPlanner() {
    // 初始化随机数种子，这里可以留空
}

void RRTPlanner::addObstacle(const SphereObstacle& obs) {
    m_obstacles.push_back(obs);
}

// 碰撞检测 (复用你之前验证过的逻辑)
bool RRTPlanner::checkCollision(const cv::Point3f& p1, const cv::Point3f& p2, float threshold) {
    for (const auto& obs : m_obstacles) {
        float safeRadius = obs.radius + threshold;
        cv::Point3f d = p2 - p1;
        cv::Point3f f = p1 - obs.center;
        float a = d.dot(d);
        float b = 2.0f * f.dot(d);
        float c = f.dot(f) - safeRadius * safeRadius;

        if (std::abs(a) < 1e-6) {
            if (c < 0) return true;
            continue;
        }

        float delta = b*b - 4*a*c;
        if (delta < 0) continue;

        delta = std::sqrt(delta);
        float t1 = (-b - delta) / (2*a);
        float t2 = (-b + delta) / (2*a);

        if (t1 <= 1.0f && t2 >= 0.0f) return true;
    }
    return false;
}

// ================= RRT 核心实现 =================

std::vector<cv::Point3f> RRTPlanner::planPath(const cv::Point3f& start, const cv::Point3f& goal) {
    std::vector<Node> tree;
    tree.push_back({start, -1}); // 1. 把起点加入树，它是根节点 (-1)

    // 随机数引擎
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    bool reached = false;
    int goalNodeId = -1;

    // 2. 开始循环生长
    for (int i = 0; i < m_maxIter; ++i) {
        // A. 采样: 有一定概率直接选终点作为方向 (Goal Bias)
        cv::Point3f rndPoint;
        if (dis(gen) < m_goalBias) {
            rndPoint = goal;
        } else {
            rndPoint = getRandomPoint(goal); // 否则全图随机撒点
        }

        // B. 找最近: 树上哪个点离这个随机点最近?
        int nearestId = getNearestNodeId(tree, rndPoint);
        cv::Point3f nearestPoint = tree[nearestId].pos;

        // C. 生长: 往那个方向迈一小步 (Step Size)
        cv::Point3f newPoint = step(nearestPoint, rndPoint);

        // D. 检测: 这一步有没有撞墙?
        if (!checkCollision(nearestPoint, newPoint)) {
            // 没撞! 加入树
            Node newNode;
            newNode.pos = newPoint;
            newNode.parentId = nearestId; // 记录父节点
            tree.push_back(newNode);

            // E. 判断: 到终点了吗? (距离小于一步长)
            if (distance(newPoint, goal) < m_stepSize) {
                reached = true;
                goalNodeId = tree.size() - 1;
                qDebug() << "✅ RRT 找到路径! 迭代次数:" << i;
                break;
            }
        }
    }

    // 3. 回溯路径 (Backtracking)
    std::vector<cv::Point3f> path;
    if (reached) {
        int currId = goalNodeId;
        path.push_back(goal); // 先放终点

        while (currId != -1) {
            path.push_back(tree[currId].pos);
            currId = tree[currId].parentId; // 找爸爸
        }
        // 现在的路径是 终点->...->起点，需要反转
        std::reverse(path.begin(), path.end());
    } else {
        qDebug() << "❌ RRT 失败: 达到最大迭代次数";
    }

    return path;
}

// --- 辅助函数实现 ---

cv::Point3f RRTPlanner::getRandomPoint(const cv::Point3f& /*goal*/) {
    // 简单的随机生成器
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(x_min, x_max);
    std::uniform_real_distribution<float> disY(y_min, y_max);
    std::uniform_real_distribution<float> disZ(z_min, z_max);
    return cv::Point3f(disX(gen), disY(gen), disZ(gen));
}

int RRTPlanner::getNearestNodeId(const std::vector<Node>& tree, const cv::Point3f& point) {
    int minId = -1;
    float minDist = std::numeric_limits<float>::max();

    for (size_t i = 0; i < tree.size(); ++i) {
        float dist = distance(tree[i].pos, point);
        if (dist < minDist) {
            minDist = dist;
            minId = i;
        }
    }
    return minId;
}

cv::Point3f RRTPlanner::step(const cv::Point3f& from, const cv::Point3f& to) {
    cv::Point3f direction = to - from;
    float len = std::sqrt(direction.dot(direction));

    // 如果距离小于步长，直接到达目标
    if (len < m_stepSize) return to;

    // 否则，归一化向量并乘以步长
    return from + direction * (m_stepSize / len);
}

float RRTPlanner::distance(const cv::Point3f& p1, const cv::Point3f& p2) {
    return cv::norm(p1 - p2);
}
