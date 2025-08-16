#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QSplitter>
#include <QTcpServer>
#include "filereceiver.h"
#include "imagepreviewlabel.h"
#include "fullscreenimagepreview.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHostAddress>
#include "imagescrollplayer.h"

// 前向声明 UI 类
namespace Ui {
class MainWindow;
}

class FileReceiver; // ✅ 前向声明 FileReceiver

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // ✅ 将 onFileReceived 移动到 public slots:
    void onFileReceived(const QString& filePath);

private slots:
    void openReceiveDirectory(); // 打开接收文件目录的槽函数
    void openSelectedImage();    // 打开选中图片进行预览的槽函数
    void refreshFileList();      // 刷新文件列表
    void previewImage(QListWidgetItem* item); // 新增槽函数，用于在点击文件列表时预览图片
    void handleDirectoryChanged(const QString& path);
    void handleReceivePathChanged(const QString& path);
    // void onHideFileListToggled(bool checked);
    void onFileListDoubleClicked(QListWidgetItem* item);
    void onBrowseReceiveButtonClicked();
    void onMessageReceived(const QString& message); // ✅ 新增槽函数
    void onNewConnection(); // ✅ 新增槽函数，用于处理新连接
    void onStartServerButtonClicked();
    void onOpenScrollPlayerClicked();
    void onScrollWindowClosed();

protected:
    // ✅ 重写事件处理函数
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateImagePreview(); // 新增函数，用于更新图片显示
    bool startServer(); // ✅ 新增的函数，封装服务器启动逻辑
    void stopServer(); // ✅ 新增的函数，封装服务器停止逻辑

private:
    Ui::MainWindow *ui;

    QString m_receiveDir;
    QFileSystemWatcher* m_watcher;
    QPixmap m_originalPixmap; // 原始图片
    QPixmap m_scaledPixmap;   // 缩放后的图片
    qreal m_scaleFactor;      // 缩放比例
    QPoint m_lastMousePos;    // 上次鼠标位置，用于拖动
    QPoint m_imageOffset;     // 图片偏移量，用于拖动

    QTcpServer* m_tcpServer;

    // ✅ 新增的成员变量，用于存储服务器配置
    QHostAddress m_serverAddress;
    quint16 m_serverPort;

    ImageScrollPlayer *m_imageScrollPlayer; // 新增滚动播放器
};

#endif // MAINWINDOW_H
