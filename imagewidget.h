#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QImage>
#include <QPolygon>

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = 0);

    void setImage(const QImage &image);
    void setImage(const QPixmap &image);
    void setCursorPosition(QPoint point);
    QPixmap pixmap();

signals:

public slots:
    void setScaleMode(bool enable);

protected:
    virtual void paintEvent(QPaintEvent * event);

private:
    bool m_scalemode;
    QImage m_image;
    QPixmap m_pixmap;
    QPoint m_point;
    QPainterPath m_cursorPath;
};

#endif // IMAGEWIDGET_H
