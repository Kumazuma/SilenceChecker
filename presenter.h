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
    QString m_ttsText;
public:
    explicit Presenter(QObject *parent = nullptr);
    QList<QAudioDeviceInfo> audioDevices();
    bool isRunning(){return m_isRunning;}
    int outCount(){return m_outCount;}
    int timeout(){return m_outTime;}
    QString ttsText(){return m_ttsText;}
protected:
    void setAudioDevice(const QAudioDeviceInfo& device);
public slots:
    void onChangedThreshold(int value);
    void setTimeout(int ms);
    void setTTS(const QString&);

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
    void changeTimeout(int);
    void overTimeout(int);
};
#endif // PRESENTER_H
