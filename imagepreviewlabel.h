#ifndef IMAGEPREVIEWLABEL_H
#define IMAGEPREVIEWLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QPoint>

class ImagePreviewLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImagePreviewLabel(QWidget *parent = nullptr);

    void setOriginalPixmap(const QPixmap& pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateScaledPixmap();

    QPixmap m_originalPixmap;
    QPixmap m_scaledPixmap;
    double m_scaleFactor;
    QPoint m_imageOffset;
    QPoint m_lastMousePos;
};

#endif // IMAGEPREVIEWLABEL_H
