#ifndef MULTICAMVIEWER_H
#define MULTICAMVIEWER_H

#include <QDialog>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <vector>

namespace Ui { class MultiCamViewer; }

class MultiCamViewer : public QDialog // 继承自 Dialog，作为弹窗运行
{
    Q_OBJECT

public:
    explicit MultiCamViewer(QWidget *parent = nullptr);
    ~MultiCamViewer();

protected:
    // 窗口显示/关闭时自动开启/停止相机，节省资源
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateCameras(); // 定时刷新函数

private:
    Ui::MultiCamViewer *ui;
    QTimer *m_timer;
    std::vector<cv::VideoCapture> m_caps; // 管理多个相机
};

#endif // MULTICAMVIEWER_H
