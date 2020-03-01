#ifndef PRESENTER_H
#define PRESENTER_H
#include <QAudioFormat>
#include <QObject>
#include <memory>
#include "audioinfo.h"
#include <QElapsedTimer>
#include <QTimer>

class Presenter : public QObject
{
    Q_OBJECT
private:
    std::unique_ptr<AudioInfo> m_audioInfo;
    std::unique_ptr<QAudioInput> m_audioInput;
    QList<QAudioDeviceInfo> m_devices;
    QElapsedTimer m_elapsedTimer;
    QTimer m_timer;
    int m_threshold;
    bool m_isRunning = false;
    int m_selectDevice = 0;

    bool m_isCountedOutTime = false;
    int m_outCount = 0;
    int m_outTime = 1000 * 60 * 1;//일단 1분
public:
    explicit Presenter(QObject *parent = nullptr);
    QList<QAudioDeviceInfo> audioDevices();
    bool isRunning(){return m_isRunning;}
    int outCount(){return m_outCount;}
protected:
    void setAudioDevice(const QAudioDeviceInfo& device);
public slots:
    void onChangedThreshold(int value);
    void start();
    void stop();
protected slots:
    void onAudioInfoUpdate();
    void onInputDeviceChanged(int index);
    void onTimeout();
signals:
    void updateLevel(int);
    void resetTimer();
    void tickTime(int);
    void runningState(bool);
};

#endif // PRESENTER_H
