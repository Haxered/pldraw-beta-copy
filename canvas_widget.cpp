#include "canvas_widget.hpp"
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QVBoxLayout>

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {

    scene = new QGraphicsScene(this);

    QGraphicsView *view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);
    setLayout(layout);
}

void CanvasWidget::addGraphic(QGraphicsItem *item) {
    if (item) {
        scene->addItem(item);
    }
}
