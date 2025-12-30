# 🤖 UR12e Remote Control & Intelligent Planning System

## 📖 项目简介 (Introduction)

本项目是一个高扩展性的工业机械臂控制与视觉规划系统，基于 **Qt 6 (C++)** 和 **OpenCV 4** 开发。

项目旨在实现 Universal Robots (UR) 系列机械臂的远程实时控制，集成多路视觉监控，并内嵌 **RRT (Rapidly-exploring Random Tree)** 路径规划算法。

采用 **模块化架构** 设计，利用 CMake 实现 Windows/Linux (Jetson Orin) 跨平台编译，并实现了 **核心算法与 UI 的解耦**。

## ✨ 核心特性 (Key Features)

### 🎮 运动控制 (Motion Control)

* **实时点动**: 支持笛卡尔空间 (X/Y/Z) 的实时 Jog 控制，基于 `speedl` 速度接口。
* **安全机制**: 实现“按住即动，松开即停”的安全逻辑 (`stopl`)。
* **网络鲁棒性**: 针对 Windows 非实时环境优化 TCP 通信，强制直连 (NoProxy) 以规避系统代理干扰。

### 🧠 智能规划 (Path Planning)

* **RRT 算法引擎**: 内置手写的 RRT 避障路径规划器 (`RRTPlanner`)。
* **碰撞检测**: 基于球体模型的 3D 空间碰撞检测算法。
* **轨迹优化**: 包含路径剪枝 (Pruning) 与平滑处理 (Smoothing) 逻辑。
* **独立测试**: 提供独立的算法单元测试 Target (`RRT_Test`)，方便算法调试。

### 👁️ 视觉系统 (Vision System)

* **多相机并发**: 支持 2-4 路 USB 相机同时采集与显示。
* **跨平台驱动**:
* **Windows**: 自动使用 DirectShow 后端。
* **Linux**: 自动使用 V4L2 后端，并优化 MJPG 格式以降低 USB 带宽压力。


* **数据采集**: 支持原始分辨率截图保存，用于数据集制作。

## 🛠️ 技术栈 (Tech Stack)

| 模块 | 技术选型 | 说明 |
| --- | --- | --- |
| **GUI Framework** | Qt 6.x / 5.15 | 信号槽机制，跨平台界面 |
| **Computer Vision** | OpenCV 4.x | 图像处理，相机驱动 |
| **Build System** | CMake 3.16+ | 跨平台构建，多目标管理 |
| **Language** | C++ 17 | 核心逻辑 |
| **Protocol** | TCP/IP | Port 30003 (UR Realtime) |

## 📂 项目结构 (Project Structure)

项目采用 **源码与平台隔离** 的工程结构：

```text
UR_Control/
├── src/
│   ├── platform/            # [跨平台层] 隔离 OS 差异代码
│   │   ├── win/             # Windows 特定实现 (DirectShow)
│   │   └── linux/           # Linux 特定实现 (V4L2)
│   ├── tests/               # [测试层] 算法单元测试入口
│   ├── RRTPlanner.h/.cpp    # [算法层] RRT 路径规划核心
│   ├── mainwindow.cpp       # [业务层] UI 与 交互逻辑
│   └── ...
├── CMakeLists.txt           # CMake 构建配置 (自动识别 OS)
├── mock_ur.py               # UR 机械臂仿真服务器 (Python)
└── README.md

```

## 🚀 构建与运行 (Build & Run)

### 1. 编译主程序 (UR_Control)

适用于 Windows (Qt Creator) 或 Linux (命令行)。

```bash
mkdir build && cd build
cmake ..
make -j4       # Windows 下使用 nmake 或 jom
./UR_Control   # 启动主界面

```

### 2. 运行算法单元测试 (RRT_Test) [New]

本项目包含独立的算法测试模块，用于验证 RRT 规划与碰撞检测逻辑，无需连接机械臂即可运行。

**在 Qt Creator 中：**

1. 点击左侧 **运行配置 (Run Settings)**（电脑图标）。
2. 在下拉菜单中选择 **`RRT_Test`**。
3. 点击运行。

**在命令行中：**

```bash
./RRT_Test
# 输出示例: ✅ 测试通过: 正确检测到碰撞!

```

## ⚠️ 常见问题与工程经验 (Troubleshooting)

### Q1: 能 Ping 通机械臂，但软件提示连接失败/超时？

> **[Engineering Fix]** 这是一个典型的网络分层问题。
> * **原因**: 电脑开启了 VPN/加速器。Qt 的 `QTcpSocket` 默认会读取系统代理设置，导致发往内网机械臂 (如 `192.168.x.x`) 的请求被错误转发到了代理服务器。
> * **解决**: 本项目代码已修复此问题。我们在 `mainwindow.cpp` 中强制设置了 `m_socket->setProxy(QNetworkProxy::NoProxy)` 以确保物理直连。
> 
> 

### Q2: 虚拟机 (URSim) 连接失败？

* 检查虚拟机防火墙：`sudo ufw disable`。
* 检查 Telnet 端口：`telnet <IP> 30003`。如果出现乱码说明连通。

### Q3: 画面卡顿或黑屏？

* 这是 USB 总线带宽不足的表现。代码已针对 Linux 开启 `MJPG` 压缩格式优化。请尝试降低分辨率或更换 USB 3.0 接口。

---

### 📅 开发计划 (Roadmap)

* [x] **Phase 1**: 基础架构重构 (src分离) 与 跨平台相机适配。
* [x] **Phase 2**: 实现 Jog 点动控制与 TCP 通信优化。
* [x] **Phase 3**: RRT 算法核心实现与单元测试环境搭建。
* [ ] **Phase 4**: **(Next)** 将 RRT 轨迹转换为 URScript 并在真机执行。
* [ ] **Phase 5**: 完成手眼标定，实现基于视觉的动态避障。

---

### 👨‍💻 作者 (Author)

* **Developer**: FuKun Yang
* **Update**: 2025-12-30