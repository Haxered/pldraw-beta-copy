#ifndef REPL_WIDGET_HPP
#define REPL_WIDGET_HPP

#include <QWidget>
#include <QString>

class QLineEdit;

class REPLWidget : public QWidget {
    Q_OBJECT

public:
    explicit REPLWidget(QWidget *parent = nullptr);

    signals:
        void lineEntered(QString completeExpression);

private slots:
    void handleReturnPressed();

private:
    QLineEdit *inputLine;
    QString accumulatedInput;

    bool isBalanced(const QString &text) const;
    static QString stripComments(const QString &text);
};

#endif