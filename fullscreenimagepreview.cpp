// fullscreenimagepreview.cpp
#include "fullscreenimagepreview.h"
#include <QHBoxLayout>
#include <QApplication>

FullscreenImagePreview::FullscreenImagePreview(QWidget* parent)
    : QWidget(parent) {
    // 隐藏窗口边框，并设置为全屏
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    showFullScreen();

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(true);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_imageLabel);
    setLayout(layout);
}

void FullscreenImagePreview::setPixmap(const QPixmap& pixmap) {
    m_imageLabel->setPixmap(pixmap);
}

void FullscreenImagePreview::keyPressEvent(QKeyEvent* event) {
    // 按下 Esc 键时关闭窗口
    if (event->key() == Qt::Key_Escape) {
        close();
    }
    // 传递事件给父类以保留其他默认行为
    QWidget::keyPressEvent(event);
}

void FullscreenImagePreview::mousePressEvent(QMouseEvent* event) {
    // 点击鼠标时关闭窗口
    close();
    QWidget::mousePressEvent(event);
}
