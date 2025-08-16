#ifndef IMAGESCROLLLABEL_H
#define IMAGESCROLLLABEL_H

#include <QLabel>
#include <QPixmap>

class ImageScrollLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageScrollLabel(QWidget *parent = nullptr);
    void setImage(const QPixmap &pixmap);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateScaledImage();

    QPixmap m_originalPixmap;  // 原始图片
    QPixmap m_scaledPixmap;    // 缩放后的图片
};

#endif // IMAGESCROLLLABEL_H
