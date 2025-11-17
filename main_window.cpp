#include "main_window.hpp"
#include "message_widget.hpp"
#include "canvas_widget.hpp"
#include "repl_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {

    // Widgets
    MessageWidget *messageWidget = new MessageWidget(this);
    CanvasWidget *canvasWidget = new CanvasWidget(this);
    REPLWidget *replWidget = new REPLWidget(this);

    // Labels
    auto *messageLabel = new QLabel(tr("Message:"), this);
    auto *replLabel    = new QLabel(tr("poslisp>"), this);

    messageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    replLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto *messageRow = new QHBoxLayout();
    messageRow->addWidget(messageLabel);
    messageRow->addWidget(messageWidget, /*stretch*/ 1);

    auto *replRow = new QHBoxLayout();
    replRow->addWidget(replLabel);
    replRow->addWidget(replWidget, /*stretch*/ 1);

    // Main vertical layout
    auto *layout = new QVBoxLayout(this);
    layout->addLayout(messageRow);
    layout->addWidget(canvasWidget);
    layout->addLayout(replRow);
    setLayout(layout);

    // REPL line entered
    connect(replWidget, &REPLWidget::lineEntered,
            &interp, &QtInterpreter::parseAndEvaluate);

    // QtInterpreter info/error
    connect(&interp, &QtInterpreter::info,
            messageWidget, &MessageWidget::info);
    connect(&interp, &QtInterpreter::error,
            messageWidget, &MessageWidget::error);

    // QtInterpreter draw graphic
    connect(&interp, &QtInterpreter::drawGraphic,
            canvasWidget, &CanvasWidget::addGraphic);
}

MainWindow::MainWindow(std::string filename, QWidget *parent) : MainWindow(parent) {

    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    interp.parseAndEvaluate(QString::fromStdString(content));
}
