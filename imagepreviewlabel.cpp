// imagepreviewlabel.cpp
#include "imagepreviewlabel.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>

ImagePreviewLabel::ImagePreviewLabel(QWidget *parent)
    : QLabel(parent)
    , m_scaleFactor(1.0)
{
    setMouseTracking(true); // 确保即使鼠标没有按住，也能接收移动事件
}

void ImagePreviewLabel::setOriginalPixmap(const QPixmap& pixmap)
{
    m_originalPixmap = pixmap;
    m_scaleFactor = 1.0;
    m_imageOffset = QPoint(0, 0);
    updateScaledPixmap();
    update(); // 强制重绘
}

void ImagePreviewLabel::paintEvent(QPaintEvent *event)
{
    if (m_scaledPixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(m_imageOffset, m_scaledPixmap);
}

void ImagePreviewLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();
    }
}

void ImagePreviewLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_scaledPixmap.isNull()) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_imageOffset += delta;
        m_lastMousePos = event->pos();
        update(); // 强制重绘
    }
}

void ImagePreviewLabel::wheelEvent(QWheelEvent *event)
{
    if (!m_scaledPixmap.isNull()) {
        int delta = event->angleDelta().y();
        if (delta > 0) {
            m_scaleFactor *= 1.1;
        } else {
            m_scaleFactor /= 1.1;
        }

        m_scaleFactor = qBound(0.1, m_scaleFactor, 10.0);

        updateScaledPixmap();
        update();
    }
}

void ImagePreviewLabel::resizeEvent(QResizeEvent *event)
{
    updateScaledPixmap();
    QLabel::resizeEvent(event);
}

void ImagePreviewLabel::updateScaledPixmap()
{
    if (m_originalPixmap.isNull()) {
        m_scaledPixmap = QPixmap();
        return;
    }
    m_scaledPixmap = m_originalPixmap.scaled(m_originalPixmap.size() * m_scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
