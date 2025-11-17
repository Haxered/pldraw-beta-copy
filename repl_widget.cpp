#include "repl_widget.hpp"
#include <QLineEdit>
#include <QVBoxLayout>

REPLWidget::REPLWidget(QWidget *parent)
    : QWidget(parent), accumulatedInput("") {
    inputLine = new QLineEdit(this);
    connect(inputLine, &QLineEdit::returnPressed,
            this, &REPLWidget::handleReturnPressed);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(inputLine);
    setLayout(layout);
}

void REPLWidget::handleReturnPressed() {
    QString currentLine = inputLine->text();
    inputLine->clear();

    // ignore empty lines
    if (currentLine.trimmed().isEmpty()) {
        return;
    }

    // accumulate with new line
    accumulatedInput += currentLine + "\n";

    // Check for balanced parentheses, after stripping comments
    QString toCheck = stripComments(accumulatedInput);
    if (isBalanced(toCheck)) {
        emit lineEntered(accumulatedInput);
        accumulatedInput.clear();
    }
    // Otherwise: incomplete, wait for more input TODO: double check if this is correct
}

bool REPLWidget::isBalanced(const QString &text) const {
    int depth = 0;

    for (QChar ch: text) {
        if (ch == '(') {
            ++depth;
        } else if (ch == ')') {
            --depth;
            if (depth < 0) return false; // Too many closing parens
        }
    }

    return depth == 0;
}

QString REPLWidget::stripComments(const QString &text) {
    QString result;
    bool inComment = false;

    for (QChar ch: text) {
        if (inComment) {
            if (ch == '\n') {
                inComment = false;
                result += '\n'; // keep \n
            }
        } else {
            if (ch == ';') {
                inComment = true;
            } else {
                result += ch;
            }
        }
    }

    return result;
}
