#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Fitness");
    QApplication::setApplicationName("Fitness");
    MainWindow window;
    window.show();
    return app.exec();
}
