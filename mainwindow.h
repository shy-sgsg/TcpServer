#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QSplitter>
#include "imagepreviewlabel.h"


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

protected:
    // ✅ 重写事件处理函数
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void createWidgets();
    void createLayout();
    void updateImagePreview(); // 新增函数，用于更新图片显示

    QString m_receiveDir;
    QListWidget* m_fileListWidget;
    ImagePreviewLabel* m_imagePreviewLabel;

    // ✅ 将按钮声明为成员变量
    QPushButton* m_openDirButton;
    QPushButton* m_refreshButton;

    QFileSystemWatcher* m_watcher;

    // ✅ 新增成员变量用于管理图片状态
    QPixmap m_originalPixmap; // 原始图片
    QPixmap m_scaledPixmap;   // 缩放后的图片
    qreal m_scaleFactor;      // 缩放比例
    QPoint m_lastMousePos;    // 上次鼠标位置，用于拖动
    QPoint m_imageOffset;     // 图片偏移量，用于拖动
};

#endif // MAINWINDOW_H
