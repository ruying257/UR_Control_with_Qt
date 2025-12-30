#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/CameraHelper.h"
#include <QTcpSocket>
#include <QMessageBox>       // 用于展示信息框
#include <QDateTime>         // 用于生成唯一的文件名
#include <QDebug>
#include <QNetworkProxy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化Socket
    m_socket = new QTcpSocket(this);
    // [Engineering Fix] 2025-12-30
    // 问题：开启系统代理(VPN)时，QTcpSocket 会尝试通过代理服务器连接内网 IP，导致连接超时。
    // 现象：能 Ping 通 (ICMP 协议不走代理)，但 TCP 连接失败。
    // 解决：强制设置 NoProxy，确保 Socket 直连物理网卡。
    m_socket->setProxy(QNetworkProxy::NoProxy);

    // TCP通信信号与槽连接
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);
    // 机械臂控制信号与槽连接
    connect(ui->btn_X_Plus, &QPushButton::pressed, this, [=](){ onJogBtnPressed(0, 1); });
    connect(ui->btn_X_Plus, &QPushButton::released, this, &MainWindow::onJogBtnReleased);
    connect(ui->btn_X_Minus, &QPushButton::pressed, this, [=](){ onJogBtnPressed(0, -1); });
    connect(ui->btn_X_Minus, &QPushButton::released, this, &MainWindow::onJogBtnReleased);
    connect(ui->btn_Y_Plus, &QPushButton::pressed, this, [=](){ onJogBtnPressed(1, 1); });
    connect(ui->btn_Y_Plus, &QPushButton::released, this, &MainWindow::onJogBtnReleased);
    connect(ui->btn_Y_Minus, &QPushButton::pressed, this, [=](){ onJogBtnPressed(1, -1); });
    connect(ui->btn_Y_Minus, &QPushButton::released, this, &MainWindow::onJogBtnReleased);
    connect(ui->btn_Z_Plus, &QPushButton::pressed, this, [=](){ onJogBtnPressed(2, 1); });
    connect(ui->btn_Z_Plus, &QPushButton::released, this, &MainWindow::onJogBtnReleased);
    connect(ui->btn_Z_Minus, &QPushButton::pressed, this, [=](){ onJogBtnPressed(2, -1); });
    connect(ui->btn_Z_Minus, &QPushButton::released, this, &MainWindow::onJogBtnReleased);

    // 初始化界面状态
    ui->lbl_Status->setText("未连接");
    ui->lbl_Status->setStyleSheet("color:red;");

    ui->lbl_Cam1->setStyleSheet("QLabel { background-color: black; }");
    ui->lbl_Cam2->setStyleSheet("QLabel { background-color: black; }");
    ui->lbl_Cam3->setStyleSheet("QLabel { background-color: black; }");
    ui->lbl_Cam4->setStyleSheet("QLabel { background-color: black; }");

    // 初始化相机
    int cameraCount = 4;

    for(int i = 0; i < cameraCount; i++){
        cv::VideoCapture cap = createCamera(i);
        if(cap.isOpened()){
            // 打印最终实际获取到的分辨率 (用于验证)
            double actualW = cap.get(cv::CAP_PROP_FRAME_WIDTH);
            double actualH = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
            qDebug() << "✅ 相机" << i << "初始化成功 | 分辨率:" << actualW << "x" << actualH;

            m_cams.push_back(cap);
        } else {
            m_cams.push_back(cv::VideoCapture());     // 即使打不开，也要压入一个空对象占位，防止后面数组越界
        } 

        m_currentFrames.push_back(cv::Mat());
    }

    // 启动定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateFrames);
    m_timer->start(33); // 33ms ≈ 30 FPS
}

MainWindow::~MainWindow()
{
    // 程序关闭前释放相机资源
    for(auto &cap : m_cams){
        if(cap.isOpened()) cap.release();
    }

    delete ui;
}

// 点击按钮槽函数
void MainWindow::on_btn_Connect_clicked()
{
    // 如果当前已经是连接状态，点击按钮意味着"断开连接"
    if(m_socket->state() == QAbstractSocket::ConnectedState){
        ui->btn_Connect->setEnabled(false); // 防止重复点击

        // m_socket->disconnectFromHost();    // 礼貌分手，等待服务器方确认才会进入UnconnectedState。现在是ClosingState

        // abort(): 拔网线，立即关闭连接，重置 Socket（快，立即进入 UnconnectedState）
        // abort()断开后，程序会立即执行onSocketDisconnected()，所以要先改变UI文本，在执行断开操作
        ui->lbl_Status->setText("正在断开...");
        m_socket->abort();
        return;
    }

    QString ip = ui->le_IPAdress->text();
    quint16 port = 30003;

    // 输入校验
    if (ip.isEmpty()){
        QMessageBox::warning(this, "提示", "IP地址不能为空");
        return;
    }

    ui->lbl_Status->setText("正在连接...");
    ui->btn_Connect->setEnabled(false);     // 连接过程中禁用按钮，防止狂点

    // 发起连接
    m_socket->connectToHost(ip, port);
}

// 连接成功槽函数
void MainWindow::onSocketConnected()
{
    ui->lbl_Status->setText("连接成功");
    ui->lbl_Status->setStyleSheet("color: green; font-weight: bold;");

    ui->btn_Connect->setText("断开连接");
    ui->btn_Connect->setEnabled(true);      // 按钮可以按下
}

// 断开连接槽函数
void MainWindow::onSocketDisconnected()
{
    ui->lbl_Status->setText("已断开");
    ui->lbl_Status->setStyleSheet("color: red;");

    ui->btn_Connect->setText("连接");
    ui->btn_Connect->setEnabled(true);
}

// 连接错误槽函数
void MainWindow::onSocketError(QAbstractSocket::SocketError socketError)
{
    // 获取可读的错误信息
    QString errorMsg = m_socket->errorString();

    ui->lbl_Status->setText("错误: " + errorMsg);
    ui->lbl_Status->setStyleSheet("color: darkred;");

    ui->btn_Connect->setText("连接机械臂");
    ui->btn_Connect->setEnabled(true);

    // 只有非断开引起的错误才弹窗，体验更好
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        QMessageBox::critical(this, "连接错误", "无法连接到机械臂:\n" + errorMsg);
    }
}

// 定时刷新逻辑
void MainWindow::updateFrames()
{
    // 将 UI 上的标签放入数组，方便循环操作
    QLabel* displayLabels[] = {ui->lbl_Cam1, ui->lbl_Cam2, ui->lbl_Cam3, ui->lbl_Cam4};

    // 遍历所有已管理的相机
    for(size_t i = 0; i < m_cams.size(); i++) {

        if(m_cams[i].isOpened()) {
            cv::Mat frame;
            m_cams[i] >> frame; // 抓取一帧

            if(!frame.empty()) {
                // 1. 存入缓存（必须存原始 BGR 数据，用于保存图片）
                m_currentFrames[i] = frame.clone();

                // 2. 转换并显示
                QImage qimg = matToQImage(frame);
                displayLabels[i]->setPixmap(QPixmap::fromImage(qimg));
            }
        }
    }
}

// 辅助函数：Mat (OpenCV) -> QImage (Qt)
QImage MainWindow::matToQImage(const cv::Mat &mat)
{
    // 1. 颜色转换 BGR -> RGB
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);

    // 2. 构造 QImage
    QImage img((const uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);

    // 3. 重要：深拷贝
    // 必须 copy()，因为 rgb 是局部变量，函数结束后内存会被释放
    return img.copy();
}

// 截图保存按钮
void MainWindow::on_btn_Capture_clicked()
{
    // 使用当前时间生成文件名，精确到秒
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    bool savedAny = false;

    for(size_t i = 0; i < m_currentFrames.size(); i++) {
        if(!m_currentFrames[i].empty()) {
            // 文件名示例: Cam1_20251217_203000.jpg
            QString filename = QString("Cam%1_%2.jpg").arg(i+1).arg(timestamp);

            // 使用 OpenCV 保存图片 (质量好，且兼容性强)
            // 注意：imwrite 需要 std::string
            cv::imwrite(filename.toStdString(), m_currentFrames[i]);

            qDebug() << "已保存:" << filename;
            savedAny = true;
        }
    }

    if(savedAny) {
        // 状态栏提示一下即可，不弹窗打扰操作
        ui->lbl_Status->setText("截图已保存至运行目录");
    } else {
        QMessageBox::warning(this, "警告", "当前没有图像数据，无法保存！");
    }
}

// 机械臂控制
// 1. 指令发送函数
void MainWindow::sendURScript(QString cmd)
{
    if(m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "⚠️ 未连接机械臂，指令发送失败";
        return;
    }

    // URScript 必须以换行符 '\n' 结尾，否则机器不执行
    cmd.append("\n");

    QByteArray data = cmd.toUtf8();
    qint64 bytesWritten = m_socket->write(data);

    if(bytesWritten == -1) {
        qDebug() << "⚠️ 写入失败";
        return;
    } else if(bytesWritten != data.size()) {
        qDebug() << "⚠️ 写入部分数据，实际写入" << bytesWritten << "字节";
        return;
    } else {
        m_socket->flush();  // 确保立即发送缓冲区数据
    }
}

// 2. 实现按下按钮（开始移动）
// axis: 0=X, 1=Y, 2=Z
// direction: 1 or -1
void MainWindow::onJogBtnPressed(int axis, int direction)
{
    // 构建速度向量 [Vx, Vy, Vz, Rx, Ry, Rz]
    double speeds[6] = {0, 0, 0, 0, 0, 0};
    speeds[axis] = direction * MOVE_VEL;

    // 拼接 URScript 字符串: speedl([x,y,z,rx,ry,rz], a, t)
    // t 设置为 100秒，意味着“一直动下去”，直到发 stopl
    QString script = QString("speedl([%1, %2, %3, 0, 0, 0], %4, 100)")
                         .arg(speeds[0]).arg(speeds[1]).arg(speeds[2])
                         .arg(MOVE_ACC);

    qDebug() << "📤 发送指令:" << script;
    sendURScript(script);
}

// 3. 实现松开按钮（立即停止）
void MainWindow::onJogBtnReleased()
{
    // stopl(a): 线性停止
    QString script = QString("stopl(%1)").arg(MOVE_ACC);

    qDebug() << "🛑 发送停止";
    sendURScript(script);
}




