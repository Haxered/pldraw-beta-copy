#include "qt_interpreter.hpp"
#include "qgraphics_arc_item.hpp"
#include "interpreter_semantic_error.hpp"
#include "tokenizer.hpp"

#include <QPen>
#include <QGraphicsEllipseItem>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

QtInterpreter::QtInterpreter(QObject *parent) : QObject(parent), Interpreter() {
}

void QtInterpreter::parseAndEvaluate(QString entry) {
    // 1) Parse the full program once
    std::istringstream iss(entry.toStdString());
    if (!parse(iss)) {
        emit error(QString("Error: Invalid Expression. Could not parse."));
        return;
    }

    try {
        // 3) Evaluate full program
        clearPendingDraws();
        Expression result = eval();

        // 4) Render graphics collected in eval
        for (const auto &graphic: getPendingDraws()) {
            createGraphicItem(graphic);
        }

        std::ostringstream oss;
        oss << result;
        emit info(QString::fromStdString(oss.str()));
    } catch (const InterpreterSemanticError &e) {
        emit error(QString("Error: ") + QString(e.what()));
    } catch (const std::exception &e) {
        emit error(QString("Error: ") + QString(e.what()));
    }
}

void QtInterpreter::createGraphicItem(const Expression &exp) {
    const Type type = exp.headType();

    if (type == PointType) {
        const Point p = exp.headValue().point_value;
        auto *item = new QGraphicsEllipseItem(p.x - 2, p.y - 2, 4, 4);
        item->setBrush(QBrush(Qt::black));
        item->setPen(Qt::NoPen);
        emit drawGraphic(item);
        return;
    }

    if (type == LineType) {
        const Line l = exp.headValue().line_value;
        auto *item = new QGraphicsLineItem(l.start.x, l.start.y, l.end.x, l.end.y);
        item->setPen(QPen(Qt::black));
        emit drawGraphic(item);
        return;
    }

    if (type == ArcType) {
        const Arc arc = exp.headValue().arc_value;

        const double dx = (arc.start.x - arc.center.x);
        const double dy = (arc.start.y - arc.center.y);
        const double radius = std::sqrt(dx * dx + dy * dy);

        const double startAngleRad = std::atan2(-dy, dx);

        const int startAngleQt = int(startAngleRad * 180.0 / M_PI * 16.0);
        const int spanAngleQt = int(arc.angle * 180.0 / M_PI * 16.0);

        const qreal x = arc.center.x - radius;
        const qreal y = arc.center.y - radius;
        const qreal w = 2 * radius;
        const qreal h = 2 * radius;

        auto *item = new QGraphicsArcItem(x, y, w, h);
        item->setStartAngle(startAngleQt);
        item->setSpanAngle(spanAngleQt);

        QPen pen(Qt::black);
        pen.setWidthF(1.0);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        item->setPen(pen);

        emit drawGraphic(item);
        return;
    }

    if (type == RectType) {
        const Rect r = exp.headValue().rect_value;
        const double x = std::min(r.point1.x, r.point2.x);
        const double y = std::min(r.point1.y, r.point2.y);
        const double w = std::abs(r.point2.x - r.point1.x);
        const double h = std::abs(r.point2.y - r.point1.y);

        auto *item = new QGraphicsRectItem(x, y, w, h);
        item->setPen(QPen(Qt::black));
        item->setBrush(Qt::NoBrush);
        emit drawGraphic(item);
        return;
    }

    if (type == FillRectType) {
        const FillRect fr = exp.headValue().fill_rect_value;
        const double x = std::min(fr.rect.point1.x, fr.rect.point2.x);
        const double y = std::min(fr.rect.point1.y, fr.rect.point2.y);
        const double w = std::abs(fr.rect.point2.x - fr.rect.point1.x);
        const double h = std::abs(fr.rect.point2.y - fr.rect.point1.y);

        auto *item = new QGraphicsRectItem(x, y, w, h);
        item->setPen(Qt::NoPen);
        item->setBrush(QBrush(QColor(int(fr.r), int(fr.g), int(fr.b))));
        emit drawGraphic(item);
        return;
    }

    if (type == EllipseType) {
        const Ellipse e = exp.headValue().ellipse_value;
        const double x = std::min(e.rect.point1.x, e.rect.point2.x);
        const double y = std::min(e.rect.point1.y, e.rect.point2.y);
        const double w = std::abs(e.rect.point2.x - e.rect.point1.x);
        const double h = std::abs(e.rect.point2.y - e.rect.point1.y);

        auto *item = new QGraphicsEllipseItem(x, y, w, h);
        item->setPen(QPen(Qt::black));
        item->setBrush(Qt::NoBrush);
        emit drawGraphic(item);
    }
}
