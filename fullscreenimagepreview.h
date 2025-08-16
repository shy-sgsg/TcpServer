#ifndef FULLSCREENIMAGEPREVIEW_H
#define FULLSCREENIMAGEPREVIEW_H

#pragma once

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QKeyEvent>

class FullscreenImagePreview : public QWidget {
    Q_OBJECT

public:
    explicit FullscreenImagePreview(QWidget* parent = nullptr);
    void setPixmap(const QPixmap& pixmap);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* m_imageLabel;
};

#endif // FULLSCREENIMAGEPREVIEW_H
