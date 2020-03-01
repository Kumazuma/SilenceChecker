#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "presenter.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    std::shared_ptr<Presenter> m_presenter;
public:
    MainWindow(std::shared_ptr<Presenter> presenter, QWidget *parent = nullptr);
    ~MainWindow();


};
#endif // MAINWINDOW_H
