#include <QApplication>

#include "src/acquisitor.h"

#include "src/mainwindow.h"

int main(int argc, char** argv) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    Acquisitor acq;
    return acq.run("");
}
