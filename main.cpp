#include "mainwindow.h"

#include <QApplication>
#include <memory>
#include <presenter.h>
#include <websideview.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::shared_ptr<Presenter> presenter(new Presenter());
    WebSideView webSideView(presenter);
    MainWindow w(presenter);
    w.show();
    return a.exec();
}
