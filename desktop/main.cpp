#include "window.h"
#include <QApplication>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Text Echo");
    QApplication::setApplicationVersion(APP_VERSION);
    Window window;
    window.show();
    return app.exec();
}
