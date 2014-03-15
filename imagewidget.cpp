#include "imagewidget.h"

#include <QPainter>
#include <QRect>
#include <QPolygonF>
#include <QPointF>
#include <QVector>
#include <QBrush>
#include <QDebug>

#include <math.h>

ImageWidget::ImageWidget(QWidget *parent) :
    QWidget(parent), m_scalemode(false)
{
    QPolygon path;
    path << QPoint(0, 0)  << QPoint(0, 15) << QPoint(10, 10) << QPoint(0,0);
    m_cursorPath.addPolygon(path);

    //setBackgroundBrush(QBrush(Qt::black));
}

void ImageWidget::setImage(const QImage &image)
{
    setImage(QPixmap::fromImage(image));
}

void ImageWidget::setImage(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    if (!m_scalemode)
        setMinimumSize(pixmap.size());
    repaint();
}


void ImageWidget::setCursorPosition(QPoint point)
{
    if (m_point != point) {
        m_point = point;
        repaint();
    }
}

void ImageWidget::setScaleMode(bool enable)
{
    m_scalemode = enable;
    setMinimumSize(enable ? QSize(10, 10) : m_pixmap.size());
    repaint();
}

void ImageWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.fillRect(rect(), QBrush(Qt::black));

    if (m_scalemode) {
        double ratio = fmin(((double)width())/m_pixmap.width(), ((double)height())/m_pixmap.height());
        QPixmap px = m_pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(0, 0, px);
        p.translate(m_point * ratio);
    } else {
        p.drawPixmap(0, 0, m_pixmap);
        p.translate(m_point);
    }

    p.setPen(QPen(QBrush(Qt::white), 1));
    p.setBrush(QBrush(Qt::black));
    p.fillPath(m_cursorPath, QBrush(Qt::black));
    p.drawPath(m_cursorPath);
}
