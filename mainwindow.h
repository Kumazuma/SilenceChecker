#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "presenter.h"
#include <QtTextToSpeech/QTextToSpeech>
#include <memory>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    std::shared_ptr<Presenter> m_presenter;
    std::unique_ptr<QTextToSpeech> m_tts;
    bool m_ttsRepeat;
public:
    MainWindow(std::shared_ptr<Presenter> presenter, QWidget *parent = nullptr);
    ~MainWindow();
protected slots:
    void onSpinTimeoutChanged(int val);
    void onTTSVoiceIndexChanged(int index);
    void onCurrentTTSLocaleChanged(const QString& name);
    void setTTsRepeat(bool);
    void onTTSStateChanged(QTextToSpeech::State);
};
#endif // MAINWINDOW_H
