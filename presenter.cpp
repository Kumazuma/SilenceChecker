#include "presenter.h"
#include <QDebug>
Presenter::Presenter(QObject *parent) :
    QObject(parent),
    m_timer(this)
{
    m_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    const auto& defaultDevice = QAudioDeviceInfo::defaultInputDevice();
    m_devices.insert(0, defaultDevice);
    m_threshold = 50;
    m_elapsedTimer.start();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

QList<QAudioDeviceInfo> Presenter::audioDevices()
{
    return m_devices;
}

void Presenter::setAudioDevice(const QAudioDeviceInfo &device)
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    if (!device.isFormatSupported(format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        format = device.nearestFormat(format);
    }

    m_audioInfo.reset(new AudioInfo(format));
    connect(m_audioInfo.get(), &AudioInfo::update,this, &Presenter::onAudioInfoUpdate);
    m_audioInput.reset(new QAudioInput(device, format));
    m_audioInfo->start();
    m_audioInput->start(m_audioInfo.get());
}

void Presenter::onAudioInfoUpdate()
{
    int value = m_audioInfo->level() * 100;
    if(value >= m_threshold)
    {
        m_elapsedTimer.restart();
        m_isCountedOutTime = false;
    }
    else if(m_elapsedTimer.elapsed() / m_outTime && !m_isCountedOutTime)
    {
        m_isCountedOutTime = true;
        ++m_outCount;
    }
    emit updateLevel(value);
}

void Presenter::onInputDeviceChanged(int index)
{
    m_selectDevice = index;
    if(m_isRunning)
    {
        m_audioInfo->stop();
        m_audioInput->stop();
        m_audioInput->disconnect(this);
        setAudioDevice(m_devices[m_selectDevice]);
    }
}

void Presenter::onChangedThreshold(int value)
{
    m_threshold = value;
}

void Presenter::onTimeout()
{
    auto milsec = m_elapsedTimer.elapsed();
    emit tickTime(milsec / 1000);
}

void Presenter::start()
{
    if(m_isRunning)
        return;
    qDebug()<<"Presenter start!";
    m_isRunning = true;
    m_elapsedTimer.restart();
    m_timer.start(500);
    setAudioDevice(m_devices[m_selectDevice]);
    emit runningState(m_isRunning);
}

void Presenter::stop()
{
    if(!m_isRunning)
        return;
    qDebug()<<"Presenter stop!";
    m_isRunning = false;
    m_timer.stop();
    m_audioInfo->stop();
    m_audioInput->stop();
    m_audioInput->disconnect(this);
    emit runningState(m_isRunning);
}
