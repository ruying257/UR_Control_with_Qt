# 🤖 UR10e Remote Control & Vision System

## 📖 项目简介 (Introduction)

本项目是一个基于 Qt (C++) 和 OpenCV 开发的机械臂远程控制与视觉监控软件。

主要功能是实现对 Universal Robots UR12e 机械臂的远程连接控制，同时集成 多路 USB 相机 实时画面监控与图像采集功能。

项目采用 **CMake** 构建，设计为跨平台架构，支持在 **Windows (开发环境)** 和 **NVIDIA Jetson Orin (部署环境/Ubuntu)** 上运行。

## ✨ 功能特性 (Features)

- **机械臂通讯**: 基于 TCP/IP 协议连接 UR 机械臂 (Port 30003 Realtime)，支持断线检测与重连。
- **多路视觉监控**: 支持同时读取 2 路（可扩展至 4 路）USB 相机画面。
- **高清采集**: 支持 1080P/720P 高清分辨率设置，自动使用 MJPG 格式优化 USB 带宽。
- **自适应 UI**: 画面显示采用 16:9 比例自适应布局，支持窗口任意缩放。
- **图像保存**: 一键截图并保存原始分辨率图像至本地。
- **跨平台**: 代码兼容 Windows (DirectShow) 和 Linux (V4L2) 驱动后端。

## 🛠️ 依赖环境 (Prerequisites)

### 硬件

- **机械臂**: Universal Robots UR12e (或使用 Python Mock Server 模拟)
- **计算平台**: Windows PC 或 NVIDIA Jetson Orin
- **相机**: USB 2.0/3.0 工业相机或普通 Webcam

### 软件栈

| **组件**         | **Windows (Dev)** | **Linux (Jetson Deploy)** |
| ---------------- | ----------------- | ------------------------- |
| **OS**           | Windows 10/11     | Ubuntu 20.04 / 22.04      |
| **Compiler**     | MSVC 2019/2022    | GCC / G++                 |
| **Qt Version**   | Qt 6.x            | Qt 5.12+ / 5.15           |
| **OpenCV**       | 4.x (MSVC build)  | 4.x (apt install)         |
| **Build System** | CMake 3.16+       | CMake 3.10+               |

## 🚀 构建与运行 (Build & Run)

### Windows 环境 (Qt Creator)

1. 安装 Qt 6 (勾选 MSVC 编译器) 和 OpenCV。
2. 在 `CMakeLists.txt` 中修改 `OpenCV_DIR` 路径指向你的 OpenCV build 目录。
3. 使用 Qt Creator 打开 `CMakeLists.txt`。
4. 配置构建套件 (Kit) 为 **Desktop Qt 6.x.x MSVC2019/2022 64bit**。
5. 点击 **构建 (Build)** 和 **运行 (Run)**。

### Linux 环境 (Jetson Orin / Ubuntu)

1. **安装依赖**:

   Bash

   ```
   sudo apt update
   sudo apt install build-essential cmake
   sudo apt install qtbase5-dev qt5-qmake      # 安装 Qt5
   sudo apt install libopencv-dev              # 安装 OpenCV
   ```

2. **编译项目**:

   Bash

   ```
   mkdir build && cd build
   cmake ..
   make -j4  # 使用 4 线程编译
   ```

3. **运行程序**:

   Bash

   ```
   ./UR_Control
   ```

## 📂 项目结构 (Project Structure)

Plaintext

```
UR_Control/
├── CMakeLists.txt       # CMake 构建脚本 (核心配置)
├── main.cpp             # 程序入口，高分屏适配
├── mainwindow.h         # 主窗口头文件 (声明)
├── mainwindow.cpp       # 主窗口实现 (业务逻辑、TCP、OpenCV)
├── mainwindow.ui        # 界面布局文件
├── mock_ur.py           # [工具] Python 编写的 UR 机械臂模拟服务器
└── README.md            # 项目说明文档
```

## ⚠️ 常见问题 (Troubleshooting)

- **相机打不开 (Windows)**:
  - 请检查 Windows 设置 -> 隐私 -> 相机，确保“允许桌面应用访问相机”已开启。
  - 代码默认使用 `CAP_DSHOW` 后端。
- **相机打不开 (Linux)**:
  - 确保当前用户在 video 组：`sudo usermod -aG video $USER`，重启生效。
- **连接机械臂失败**:
  - 检查 IP 地址是否在同一网段。
  - 如果无真机，请运行 `python mock_ur.py` 启动本地模拟服务器，并连接 `127.0.0.1`。
- **画面卡顿/黑屏**:
  - 可能是 USB 带宽不足。尝试将相机插在不同的 USB 控制器接口上，或降低分辨率设置。
- **能够 Ping 通机械臂，但软件提示连接失败**：
  - 请检查电脑是否开启了 VPN/加速器。Qt 的网络模块默认会读取系统代理设置，导致内网请求被转发到代理服务器。请关闭代理或在代码中强制禁用代理。

------

### 👨‍💻 作者 (Author)

- **Developer**: FuKun Yang
- **Date**: 2025-12-18

------

### 💡TO DO LIST

- [ ] 实现 IBVS 视觉伺服算法
- [ ] 添加相机参数标定功能
- [ ] 优化多线程图像处理性能