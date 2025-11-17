#include "repl_widget.hpp"

#include <QVBoxLayout>
#include <QKeyEvent>

REPLWidget::REPLWidget(QWidget *parent) : QWidget(parent), historyIndex(-1) {
    inputLine = new QLineEdit(this);
    inputLine->installEventFilter(this);
    connect(inputLine, &QLineEdit::returnPressed, this, &REPLWidget::changed);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(inputLine);
    setLayout(layout);
}

void REPLWidget::changed() {
    QString entry = inputLine->text();

    if (!entry.isEmpty()) {
        history.removeAll(entry);
        history.prepend(entry);

        emit lineEntered(entry);
    }

    inputLine->clear();

    historyIndex = -1;
    currentInput = "";
}

bool REPLWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == inputLine && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Up) {
            if (history.isEmpty()) {
                return true;
            }

            if (historyIndex == -1) {
                currentInput = inputLine->text();
            }

            if (historyIndex < history.size() - 1) {
                historyIndex++;
                inputLine->setText(history[historyIndex]);
            }

            return true;
        }
        if (keyEvent->key() == Qt::Key_Down) {
            if (history.isEmpty() || historyIndex <= -1) {
                return true;
            }

            if (historyIndex > 0) {
                historyIndex--;
                inputLine->setText(history[historyIndex]);
            } else if (historyIndex == 0) {
                historyIndex = -1;
                inputLine->setText(currentInput);
            }

            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}
