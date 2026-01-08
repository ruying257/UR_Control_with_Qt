#include "MultiCamViewer.h"
#include "ui_MultiCamViewer.h"
#include "../platform/CameraHelper.h" // 复用你的相机驱动

MultiCamViewer::MultiCamViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiCamViewer)
{
    ui->setupUi(this);
    this->setWindowTitle("🛠️ 工具箱: 多相机监视器");

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MultiCamViewer::updateCameras);
}

MultiCamViewer::~MultiCamViewer() {
    delete ui;
}

void MultiCamViewer::showEvent(QShowEvent *event) {
    // 窗口打开时，初始化相机
    // 假设你有 4 个相机，ID 分别为 0,1,2,3
    // 注意：如果 MainWindow 占用了 ID 0，这里可能会冲突，需要做资源管理
    // 简单起见，这里假设是独立的或者是 ID 1,2,3,4
    for(int i=0; i<4; ++i) {
        m_caps.push_back(createCamera(i));
    }
    m_timer->start(30); // 30ms 刷新
    QDialog::showEvent(event);
}

void MultiCamViewer::closeEvent(QCloseEvent *event) {
    // 窗口关闭时，释放相机资源
    m_timer->stop();
    for(auto &cap : m_caps) {
        if(cap.isOpened()) cap.release();
    }
    m_caps.clear();
    QDialog::closeEvent(event);
}

void MultiCamViewer::updateCameras() {
    // 遍历读取并显示
    // 逻辑和你原来的 updateFrames 类似，只是不用做视觉识别，纯显示
    // ui->lbl_Cam1->setPixmap(...)
}
