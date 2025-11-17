#include "qgraphics_arc_item.hpp"

#include <qpainter.h>

QGraphicsArcItem::QGraphicsArcItem(qreal x, qreal y, qreal width, qreal height,
                                   QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent) {
}

void QGraphicsArcItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(pen());
    painter->setBrush(Qt::NoBrush);
    painter->drawArc(rect(), startAngle(), spanAngle());
}
