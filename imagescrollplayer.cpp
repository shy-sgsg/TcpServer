#include "imagescrollplayer.h"
#include <QPainter>
#include <QDebug>

ImageScrollPlayer::ImageScrollPlayer(QWidget *parent)
    : QWidget(parent),
    m_scrollSpeed(50),  // 默认速度50像素/秒
    m_scrollOffset(0),
    m_totalWidth(0),
    m_targetHeight(0)
{
    m_scrollTimer = new QTimer(this);
    // 每30毫秒更新一次，约33帧/秒
    connect(m_scrollTimer, &QTimer::timeout, this, &ImageScrollPlayer::updateScrollPosition);

    // 设置窗口大小
    resize(1024, 768);
    setWindowTitle("图片滚动播放");
}

void ImageScrollPlayer::addNewImage(const QString &imagePath)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        // 计算图片应显示的尺寸（按窗口高度等比例缩放）
        if (m_targetHeight == 0) {
            m_targetHeight = height() * 0.9; // 默认为窗口高度的90%
        }

        QSize scaledSize = pixmap.size();
        scaledSize.scale(QWIDGETSIZE_MAX, m_targetHeight, Qt::KeepAspectRatio);

        m_images.append(pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_imageWidths.append(scaledSize.width());
        m_totalWidth += scaledSize.width();

        adjustImagePositions();
        update();
    } else {
        qDebug() << "无法加载图片:" << imagePath;
    }
}

void ImageScrollPlayer::setScrollSpeed(int speed)
{
    m_scrollSpeed = speed;
}

void ImageScrollPlayer::toggleScroll(bool start)
{
    if (start && !m_scrollTimer->isActive()) {
        m_scrollTimer->start(30);
    } else if (!start && m_scrollTimer->isActive()) {
        m_scrollTimer->stop();
    }
}

void ImageScrollPlayer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制背景
    painter.fillRect(rect(), Qt::black);

    // 绘制所有图片
    for (int i = 0; i < m_images.size(); ++i) {
        const QPixmap &pix = m_images[i];
        const QPoint &pos = m_imagePositions[i];

        // 只绘制在窗口可见区域内的图片
        if (pos.x() + pix.width() > 0 && pos.x() < width()) {
            painter.drawPixmap(pos, pix);
        }
    }
}

void ImageScrollPlayer::resizeEvent(QResizeEvent *event)
{
    m_targetHeight = height() * 0.9;
    // 重新计算所有图片尺寸
    m_totalWidth = 0;
    m_imageWidths.clear();

    for (int i = 0; i < m_images.size(); ++i) {
        QSize scaledSize = m_images[i].size();
        scaledSize.scale(QWIDGETSIZE_MAX, m_targetHeight, Qt::KeepAspectRatio);
        m_imageWidths.append(scaledSize.width());
        m_totalWidth += scaledSize.width();
        m_images[i] = m_images[i].scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    adjustImagePositions();
    QWidget::resizeEvent(event);
}

void ImageScrollPlayer::updateScrollPosition()
{
    if (m_images.isEmpty()) return;

    // 计算每帧滚动的距离
    qreal distancePerFrame = (m_scrollSpeed / 1000.0) * 30;
    m_scrollOffset += distancePerFrame;

    // 当滚动超过总宽度时，重置（可选：根据需求决定是否循环）
    // if (m_scrollOffset >= m_totalWidth - width()) {
    //     m_scrollOffset = 0; // 循环滚动
    // }

    adjustImagePositions();
    update();
}

void ImageScrollPlayer::adjustImagePositions()
{
    if (m_images.isEmpty()) return;

    m_imagePositions.clear();
    qreal currentX = -m_scrollOffset;

    // 依次排列图片，实现无缝拼接
    for (int i = 0; i < m_images.size(); ++i) {
        // 计算图片Y坐标（垂直居中）
        int yPos = (height() - m_images[i].height()) / 2;
        m_imagePositions.append(QPoint(currentX, yPos));
        currentX += m_imageWidths[i];
    }
}
