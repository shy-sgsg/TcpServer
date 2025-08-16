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
#include <QFileDialog>
#include "ui_mainwindow.h"
#include <QTcpServer>
#include "filereceiver.h"
#include <QLineEdit> // 新增
#include <QLabel> // 新增

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scaleFactor(1.0)
    , m_imageOffset(0, 0)
    , m_watcher(new QFileSystemWatcher(this))
    , m_tcpServer(new QTcpServer(this))
{
    ui->setupUi(this); // ✅ 设置界面

    setWindowTitle("文件接收服务器");
    setMinimumSize(800, 600);

    ui->serverAddressLineEdit->setPlaceholderText("请输入要监听的 IP 地址");
    ui->serverPortLineEdit->setPlaceholderText("请输入要监听的端口号");
    ui->receivePathLineEdit->setPlaceholderText("请输入接收文件保存的路径");

    m_receiveDir = "E:/AIR/小长ISAR/实时数据回传/receive";
    ui->receivePathLineEdit->setText(m_receiveDir);
    ui->serverAddressLineEdit->setText("0.0.0.0");
    ui->serverPortLineEdit->setText("65432");

    // ✅ 找到并连接新的 UI 控件
    connect(ui->startServerButton, &QPushButton::clicked, this, &MainWindow::onStartServerButtonClicked);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    // ... (保持你原有的信号和槽连接不变)
    connect(ui->fileListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onFileListDoubleClicked);
    connect(ui->fileListWidget, &QListWidget::itemClicked, this, &MainWindow::previewImage);
    connect(ui->browseReceiveButton, &QPushButton::clicked, this, &MainWindow::onBrowseReceiveButtonClicked);
    connect(ui->receivePathLineEdit, &QLineEdit::textChanged, this, &MainWindow::handleReceivePathChanged);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::handleDirectoryChanged);

    // ✅ 初始启动服务器
    startServer();

    m_watcher->addPath(m_receiveDir);
    setMouseTracking(true);
    refreshFileList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ✅ 新增的服务器启动/重启槽函数
void MainWindow::onStartServerButtonClicked()
{
    // 停止旧服务器（如果有的话）
    stopServer();

    // 从 UI 控件中读取新的配置
    QString addressText = ui->serverAddressLineEdit->text();
    QString portText = ui->serverPortLineEdit->text();

    m_serverAddress = QHostAddress(addressText);
    m_serverPort = portText.toUInt();

    // 检查地址和端口是否合法
    if (m_serverAddress.isNull() || m_serverPort == 0) {
        QMessageBox::critical(this, "错误", "监听地址或端口号无效，请检查输入。");
        return;
    }

    // 尝试启动新服务器
    if (startServer()) {
        QMessageBox::information(this, "成功", QString("服务器已成功启动，正在监听 %1:%2").arg(m_serverAddress.toString()).arg(m_serverPort));
    }
}

// ✅ 封装的服务器停止函数
void MainWindow::stopServer()
{
    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
        onMessageReceived("服务器已停止监听。");
    }
}

// ✅ 新增的槽函数，用于处理接收到的消息
void MainWindow::onMessageReceived(const QString& message)
{
    if (ui->messageLogTextEdit) {
        ui->messageLogTextEdit->append(
            QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(message)
            );
        // 自动滚动到底部
        QScrollBar* sb = ui->messageLogTextEdit->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

// ✅ 封装的服务器启动函数
bool MainWindow::startServer()
{
    if (m_tcpServer->isListening()) {
        stopServer();
    }

    if (!m_tcpServer->listen(m_serverAddress, m_serverPort)) {
        QMessageBox::critical(this, "错误", "无法启动服务器：" + m_tcpServer->errorString());
        return false;
    }

    onMessageReceived(QString("服务器已在 %1:%2 上启动。").arg(m_serverAddress.toString()).arg(m_serverPort));
    return true;
}

// ✅ 核心修改：处理新连接
void MainWindow::onNewConnection()
{
    QTcpSocket* socket = m_tcpServer->nextPendingConnection();
    if (socket) {
        qDebug() << "新客户端已连接：" << socket->peerAddress().toString();

        FileReceiver* receiver = new FileReceiver(socket, this);

        // ✅ 关键：连接 FileReceiver 的信号到 MainWindow 的槽函数
        connect(receiver, &FileReceiver::fileReceived, this, &MainWindow::onFileReceived);
        connect(receiver, &FileReceiver::messageReceived, this, &MainWindow::onMessageReceived); // ✅ 消息信号连接
        connect(receiver, &FileReceiver::finished, receiver, &FileReceiver::deleteLater); // ✅ 连接 finished 信号，接收器会自动销毁
    }
}

// 确保这个函数定义在你的 mainwindow.cpp 文件中
void MainWindow::handleReceivePathChanged(const QString& newPath)
{
    m_receiveDir = newPath;
    // 移除旧路径的监控，添加新路径
    m_watcher->removePaths(m_watcher->directories());
    m_watcher->addPath(m_receiveDir);
    refreshFileList();
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

void MainWindow::onFileListDoubleClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }

    QString filePath = QDir(m_receiveDir).filePath(item->text());

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片文件或文件不是图片格式。");
        return;
    }

    // 创建并显示全屏预览窗口
    FullscreenImagePreview* preview = new FullscreenImagePreview(this);
    preview->setPixmap(pixmap);
    preview->setAttribute(Qt::WA_DeleteOnClose); // ✅ 窗口关闭时自动删除对象
    preview->show();
}

void MainWindow::onBrowseReceiveButtonClicked()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this, "选择文件保存目录", ui->receivePathLineEdit->text());
    if (!selectedDir.isEmpty()) {
        ui->receivePathLineEdit->setText(selectedDir);
    }
}

// 修改后的 previewImage() 函数
void MainWindow::previewImage(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    QString filePath = QDir(m_receiveDir).filePath(item->text());

    QPixmap pixmap;
    pixmap.load(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片文件或文件不是图片格式。");
        ui->imagePreviewLabel->clear();
        return;
    }

    ui->imagePreviewLabel->setOriginalPixmap(pixmap);
}

void MainWindow::openSelectedImage()
{
    // 这个槽函数现在可以被移除了，或者保持原样。我们已经有了 previewImage。
    // 为了保持和之前代码的兼容性，您可以将它重定向到 previewImage。
    // previewImage(m_fileListWidget->currentItem());
}


void MainWindow::refreshFileList()
{
    ui->fileListWidget->clear();
    ui->imagePreviewLabel->clear();
    ui->imagePreviewLabel->setText("图片预览");
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
        ui->fileListWidget->addItem(fileInfo.fileName());
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
    QPixmap canvas(ui->imagePreviewLabel->size());
    canvas.fill(Qt::white); // 或其他背景色

    QPainter painter(&canvas);
    // 绘制缩放后的图片，并应用偏移量
    painter.drawPixmap(m_imageOffset.x(), m_imageOffset.y(), m_scaledPixmap);
    ui->imagePreviewLabel->setPixmap(canvas);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 检查鼠标是否在 m_imagePreviewLabel 的几何区域内
    if (ui->imagePreviewLabel->geometry().contains(event->pos())) {
        if (event->button() == Qt::LeftButton) {
            // 将鼠标位置转换为 m_imagePreviewLabel 的局部坐标
            m_lastMousePos = event->pos() - ui->imagePreviewLabel->pos();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 只有当鼠标左键按下且鼠标在预览区域内时才处理
    if (event->buttons() & Qt::LeftButton && ui->imagePreviewLabel->geometry().contains(event->pos())) {
        QPoint delta = event->pos() - (m_lastMousePos + ui->imagePreviewLabel->pos());
        m_imageOffset += delta;
        m_lastMousePos = event->pos() - ui->imagePreviewLabel->pos();
        updateImagePreview();
    }
}

// ✅ 滚轮事件，用于缩放
void MainWindow::wheelEvent(QWheelEvent *event)
{
    // 检查鼠标滚轮事件是否发生在 m_imagePreviewLabel 的几何区域内
    if (ui->imagePreviewLabel->geometry().contains(event->position().toPoint())) {
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
