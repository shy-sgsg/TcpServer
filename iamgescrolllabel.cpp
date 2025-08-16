// imagescrolllabel.cpp
#include "imagescrolllabel.h"
#include <QPainter>
#include <QResizeEvent>

ImageScrollLabel::ImageScrollLabel(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
}

void ImageScrollLabel::setImage(const QPixmap &pixmap)
{
    m_originalPixmap = pixmap;
    updateScaledImage();
    update();
}

void ImageScrollLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updateScaledImage();  // 窗口大小变化时重新缩放
}

void ImageScrollLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (m_scaledPixmap.isNull()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑缩放

    // 计算居中位置
    int x = (width() - m_scaledPixmap.width()) / 2;
    int y = (height() - m_scaledPixmap.height()) / 2;

    painter.drawPixmap(x, y, m_scaledPixmap);
}

void ImageScrollLabel::updateScaledImage()
{
    if (m_originalPixmap.isNull()) {
        m_scaledPixmap = QPixmap();
        return;
    }

    // 计算适应窗口的最佳缩放比例（保持 aspect ratio）
    qreal scaleX = (qreal)width() / m_originalPixmap.width();
    qreal scaleY = (qreal)height() / m_originalPixmap.height();
    qreal scale = qMin(scaleX, scaleY);  // 取较小的比例，确保图片完全显示

    // 使用高质量缩放
    m_scaledPixmap = m_originalPixmap.scaled(
        m_originalPixmap.size() * scale,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation  // 关键：使用平滑变换
        );
}
