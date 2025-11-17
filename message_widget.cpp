#include "message_widget.hpp"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QPalette>

MessageWidget::MessageWidget(QWidget *parent) : QWidget(parent) {
    messageDisplay = new QLineEdit(this);
    messageDisplay->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(messageDisplay);
    setLayout(layout);
}

void MessageWidget::info(QString message) {
    QPalette palette = messageDisplay->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Base));
    messageDisplay->setPalette(palette);
    messageDisplay->deselect();
    messageDisplay->setText(message);
}

void MessageWidget::error(QString message) {
    messageDisplay->setText(message);

    QPalette palette = messageDisplay->palette();
    palette.setColor(QPalette::Highlight, Qt::red);
    messageDisplay->setPalette(palette);

    messageDisplay->selectAll();
}

void MessageWidget::clear() {
    QPalette palette = messageDisplay->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Base));
    messageDisplay->setPalette(palette);
    messageDisplay->deselect();
    messageDisplay->clear();
}
