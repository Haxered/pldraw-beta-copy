#include <QApplication>
#include "main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow *window;

    if (argc >= 2) {
        window = new MainWindow(std::string(argv[1]));
    } else {
        window = new MainWindow();
    }

    window->setMinimumSize(800, 600);
    window->show();
    return app.exec();
}
