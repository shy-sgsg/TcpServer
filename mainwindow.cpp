#include "mainwindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QPixmap>
#include <QScrollBar>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scaleFactor(1.0)
    , m_imageOffset(0, 0)
    , m_watcher(new QFileSystemWatcher(this))
{
    setWindowTitle("文件接收服务器");
    setMinimumSize(800, 600);

    m_receiveDir = "E:/AIR/小长ISAR/实时数据回传/receive";

    createWidgets();
    createLayout();
    refreshFileList();

    // ✅ 将接收目录添加到观察者，并连接信号
    m_watcher->addPath(m_receiveDir);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::handleDirectoryChanged);

    // 禁用默认的鼠标跟踪，我们自己处理
    setMouseTracking(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createWidgets()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QPushButton* openDirButton = new QPushButton("打开接收目录");
    QPushButton* refreshButton = new QPushButton("刷新列表");

    m_fileListWidget = new QListWidget;

    m_imagePreviewLabel = new ImagePreviewLabel(this);
    m_imagePreviewLabel->setText("图片预览"); // 初始文本
    m_imagePreviewLabel->setAlignment(Qt::AlignCenter);
    m_imagePreviewLabel->setFrameShape(QFrame::Box);
    // m_imagePreviewLabel->setScaledContents(true); // ✅ 移除，我们自己处理缩放

    connect(openDirButton, &QPushButton::clicked, this, &MainWindow::openReceiveDirectory);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshFileList);
    // ✅ 更改为点击列表项时调用新的预览函数
    connect(m_fileListWidget, &QListWidget::itemClicked, this, &MainWindow::previewImage);
}

void MainWindow::createLayout()
{
    QHBoxLayout* mainLayout = new QHBoxLayout;
    QVBoxLayout* leftLayout = new QVBoxLayout;
    QHBoxLayout* buttonLayout = new QHBoxLayout;

    // ✅ 将成员变量按钮添加到布局
    // buttonLayout->addWidget(m_openDirButton);
    // buttonLayout->addWidget(m_refreshButton);

    leftLayout->addWidget(new QLabel("接收文件列表:"));
    leftLayout->addWidget(m_fileListWidget);
    leftLayout->addLayout(buttonLayout);

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addWidget(m_imagePreviewLabel, 2);

    centralWidget()->setLayout(mainLayout);
}

void MainWindow::openReceiveDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_receiveDir));
}

// ✅ 新增的槽函数，当目录变化时自动调用
void MainWindow::handleDirectoryChanged(const QString& path)
{
    Q_UNUSED(path);
    qDebug() << "接收目录发生变化，正在刷新列表...";
    refreshFileList();
}

// 修改后的 previewImage() 函数
void MainWindow::previewImage(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    QString fileName = item->text();
    QString filePath = QDir(m_receiveDir).filePath(fileName);

    QPixmap pixmap;
    pixmap.load(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片文件或文件不是图片格式。");
        m_imagePreviewLabel->clear();
        return;
    }

    m_imagePreviewLabel->setOriginalPixmap(pixmap);
}

void MainWindow::openSelectedImage()
{
    // 这个槽函数现在可以被移除了，或者保持原样。我们已经有了 previewImage。
    // 为了保持和之前代码的兼容性，您可以将它重定向到 previewImage。
    // previewImage(m_fileListWidget->currentItem());
}


void MainWindow::refreshFileList()
{
    m_fileListWidget->clear();
    m_imagePreviewLabel->clear();
    m_imagePreviewLabel->setText("图片预览");
    m_originalPixmap = QPixmap();
    m_scaledPixmap = QPixmap();

    QDir dir(m_receiveDir);
    if (!dir.exists()) {
        QMessageBox::warning(this, "错误", "接收目录不存在，请检查路径。");
        return;
    }

    QStringList filters;
    filters << "*.jpg" << "*.png" << "*.bmp" << "*.tif";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fileInfo : fileList) {
        m_fileListWidget->addItem(fileInfo.fileName());
    }
}

void MainWindow::onFileReceived(const QString& filePath)
{
    qDebug() << "文件已接收：" << filePath << "，正在刷新列表。";
    refreshFileList();
}

// ✅ 新的更新图片预览函数
void MainWindow::updateImagePreview()
{
    if (m_originalPixmap.isNull()) {
        return;
    }

    // 根据缩放比例缩放图片
    m_scaledPixmap = m_originalPixmap.scaled(m_originalPixmap.size() * m_scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个空白图片作为画布
    QPixmap canvas(m_imagePreviewLabel->size());
    canvas.fill(Qt::white); // 或其他背景色

    QPainter painter(&canvas);
    // 绘制缩放后的图片，并应用偏移量
    painter.drawPixmap(m_imageOffset.x(), m_imageOffset.y(), m_scaledPixmap);
    m_imagePreviewLabel->setPixmap(canvas);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 检查鼠标是否在 m_imagePreviewLabel 的几何区域内
    if (m_imagePreviewLabel->geometry().contains(event->pos())) {
        if (event->button() == Qt::LeftButton) {
            // 将鼠标位置转换为 m_imagePreviewLabel 的局部坐标
            m_lastMousePos = event->pos() - m_imagePreviewLabel->pos();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 只有当鼠标左键按下且鼠标在预览区域内时才处理
    if (event->buttons() & Qt::LeftButton && m_imagePreviewLabel->geometry().contains(event->pos())) {
        QPoint delta = event->pos() - (m_lastMousePos + m_imagePreviewLabel->pos());
        m_imageOffset += delta;
        m_lastMousePos = event->pos() - m_imagePreviewLabel->pos();
        updateImagePreview();
    }
}

// ✅ 滚轮事件，用于缩放
void MainWindow::wheelEvent(QWheelEvent *event)
{
    // 检查鼠标滚轮事件是否发生在 m_imagePreviewLabel 的几何区域内
    if (m_imagePreviewLabel->geometry().contains(event->position().toPoint())) {
        if (event->buttons() & Qt::LeftButton && !m_scaledPixmap.isNull()) {
            int delta = event->angleDelta().y();
            if (delta > 0) {
                m_scaleFactor *= 1.1; // 放大
            } else {
                m_scaleFactor /= 1.1; // 缩小
            }

            // 限制缩放比例在合理范围内
            if (m_scaleFactor < 0.1) {
                m_scaleFactor = 0.1;
            }
            if (m_scaleFactor > 10.0) {
                m_scaleFactor = 10.0;
            }

            updateImagePreview();
        }
    }
}
