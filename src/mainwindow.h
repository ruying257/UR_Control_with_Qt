#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>      // 引入Socket错误枚举
#include <QTimer>               // 定时器
#include <opencv2/opencv.hpp>   // OpenCV头文件


class QTcpSocket;   // 前置声明

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    // IP通讯处理
    void on_btn_Connect_clicked();  // 按钮点击槽（Qt特有自动命名规则）
    void onSocketConnected();       // 自定义槽：处理连接成功
    void onSocketDisconnected();    // 自定义槽：处理断开
    void onSocketError(QAbstractSocket::SocketError socketError);   // 自定义槽：处理错误

    // 相机画面显示及保存
    void updateFrames();            // 定时器触发：读取并显示画面
    void on_btn_Capture_clicked();  // 按钮触发：保存图片

    // 机械臂控制
    void sendURScript(QString emd); // 通用指令发送函数

    void onJogBtnPressed(int axis, int direction);  // axis: 0=X, 1=Y, 2=Z; direction: 1=正, -1=负
    void onJogBtnReleased();

private:
    Ui::MainWindow *ui;

    // 创建指针；成员变量加 m_ 前缀，一眼看出这是成员变量，不是局部变量
    QTcpSocket *m_socket;   // TCP通讯

    // 视觉相关变量
    QTimer *m_timer;                        // 负责刷新画面的定时器
    std::vector<cv::VideoCapture> m_cams;   // 管理所有相机对象
    std::vector<cv::Mat> m_currentFrames;   // 缓存当前的原始画面（用于保存）

    // 辅助函数：将 OpenCV 的 Mat (BGR) 转为 Qt 的 QImage (RGB)
    QImage matToQImage(const cv::Mat &mat);

    // 预定义速度和加速度
    const double MOVE_ACC = 0.5;    // m/s^2
    const double MOVE_VEL = 0.1;    // m/s
};
#endif // MAINWINDOW_H
