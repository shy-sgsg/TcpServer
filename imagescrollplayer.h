#ifndef IMAGESCROLLPLAYER_H
#define IMAGESCROLLPLAYER_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <QPoint>

class ImageScrollPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit ImageScrollPlayer(QWidget *parent = nullptr);

    // 添加新图片到滚动序列
    void addNewImage(const QString &imagePath);
    // 设置滚动速度(像素/秒)
    void setScrollSpeed(int speed);
    // 开始/暂停滚动
    void toggleScroll(bool start);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateScrollPosition();

private:
    QTimer *m_scrollTimer;      // 控制滚动的定时器
    QVector<QPixmap> m_images;  // 存储所有接收的图片
    QVector<QPoint> m_imagePositions; // 存储每张图片的位置
    QVector<int> m_imageWidths; // 存储每张图片的宽度
    int m_totalWidth;           // 图片总宽度
    int m_scrollSpeed;          // 滚动速度(像素/秒)
    qreal m_scrollOffset;       // 滚动偏移量
    int m_targetHeight;         // 图片显示高度(统一高度)

    // 调整图片位置，实现无缝滚动
    void adjustImagePositions();
};

#endif // IMAGESCROLLPLAYER_H
