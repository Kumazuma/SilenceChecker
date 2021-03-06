#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QListWidget>
#include <QDebug>
template<typename T>
void addItems(const T& lists, QListWidget* widget)
{
    for(auto rate : lists)
    {
        widget->addItem(QString("%1").arg(rate));
    }
}
#define CASE_TO_STRING(x, y) case x:y=#x;break;
MainWindow::MainWindow(std::shared_ptr<Presenter> presenter, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_presenter(std::move(presenter)),
      m_ttsRepeat(false)

{
    ui->setupUi(this);
    auto engineList = QTextToSpeech::availableEngines();
    if(engineList.empty())
        return;
    auto local = QLocale(QLocale::AnyLanguage);
    m_tts.reset( new QTextToSpeech(engineList.first()));
    m_tts->setLocale(local);
    auto locales = m_tts->availableLocales();
    auto voices = m_tts->availableVoices();
    for(auto& locale : locales)
    {
        QString text = QString("%1").arg(locale.name());
        ui->cboLocale->addItem(text);
    }
    ui->cboLocale->setCurrentText(QLocale::c().name());
    if(voices.size() == 0)
        return;
    m_tts->setVoice(voices.first());
    m_tts->setVolume(1.0);

    for(auto& voice : voices)
    {
        QString text = QString("%1(%2)").arg(voice.name()).arg(voice.gender());
        ui->cboVoice->addItem(text);
    }

    connect(ui->cboVoice, SIGNAL(currentIndexChanged(int)), this, SLOT(onTTSVoiceIndexChanged(int)));
    connect(ui->cboLocale, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onCurrentTTSLocaleChanged(const QString&)));
    for (auto &deviceInfo: m_presenter->audioDevices()) {
         QString name = deviceInfo.deviceName();
        ui->comboBox->addItem(name);
    }
    connect(m_presenter.get(), SIGNAL(updateLevel(int)),ui->lcdNumber, SLOT(display(int)));
    connect(ui->horizontalSlider, &QSlider::valueChanged, m_presenter.get(), &Presenter::onChangedThreshold);
    connect(ui->btnStart, &QPushButton::clicked, [this](bool)
    {
        this->m_presenter->start();
    });
    connect(ui->btnStop, &QPushButton::clicked, [this](bool)
    {
        this->m_presenter->stop();
    });
    connect(m_presenter.get(), &Presenter::runningState, [this](bool running)
    {
        ui->btnStop->setEnabled(running);
        ui->btnStart->setEnabled(!running);
    });

    connect(m_presenter.get(), &Presenter::tickTime,[this](int ms)->void
    {
        int msec = (ms % 1000) / 10;
        int sec = (ms / 1000) % 60;
        int min = (ms / 60000);
        ui->lcdSec->display(msec);
        ui->lcdMin->display(sec);
        ui->lcdHour->display(min);
    });
    connect(m_presenter.get(), &Presenter::overTimeout,[this](int)->void
    {
        auto ttsText = m_presenter->ttsText().trimmed();
        if(ttsText.size() == 0)
            return;
        m_tts->say(ttsText);
    });
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),m_presenter.get(), SLOT(onInputDeviceChanged(int)));
    connect(ui->spinTimeout, SIGNAL(valueChanged(int)), this, SLOT(onSpinTimeoutChanged(int)));
    connect(ui->txtTTSText, SIGNAL(textEdited(const QString&)), m_presenter.get(), SLOT(setTTS(const QString&)));
    connect(ui->chkRepeat, SIGNAL(clicked(bool)), this,SLOT(setTTsRepeat(bool)));
    connect(m_tts.get(), SIGNAL(stateChanged(QTextToSpeech::State)), this, SLOT(onTTSStateChanged(QTextToSpeech::State)));
    connect(m_tts.get(), SIGNAL(voiceChanged(const QVoice &)), this, SLOT(onTTSVoiceChanged(const QVoice&)));
    //connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),m_presenter.get(), SLOT(onInputDeviceChanged(int)));

//    auto sampleRates = defaultDeviceInfo.supportedSampleRates();
//    auto sampleSizes = defaultDeviceInfo.supportedSampleSizes();
//    auto sampleTypes = defaultDeviceInfo.supportedSampleTypes();
//    auto channelCounts = defaultDeviceInfo.supportedChannelCounts();
//    std::vector<QString> types;
//    for(auto type:sampleTypes)
//    {
//        QString text;
//        switch(type)
//        {
//        CASE_TO_STRING(QAudioFormat::SampleType::Float, text)
//        CASE_TO_STRING(QAudioFormat::SampleType::Unknown, text)
//        CASE_TO_STRING(QAudioFormat::SampleType::SignedInt, text)
//        CASE_TO_STRING(QAudioFormat::SampleType::UnSignedInt, text)
//        }
//        types.push_back(text);
//    }
//    addItems(sampleRates, ui->listWidget);
//    addItems(sampleSizes, ui->listWidget_2);
//    addItems(types, ui->listWidget_3);
//    addItems(channelCounts, ui->listWidget_4);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSpinTimeoutChanged(int val)
{
    int ms = val * 1000 * 60;
    if(ms != m_presenter->timeout())
        m_presenter->setTimeout(ms);
}

void MainWindow::onTTSVoiceIndexChanged(int index)
{
    if(index == -1)
        return;
    auto voice = m_tts->availableVoices()[index];
    m_tts->setVoice(voice);
}

void MainWindow::onCurrentTTSLocaleChanged(const QString &name)
{
    ui->cboVoice->clear();
    QLocale locale(name);
    m_tts->setLocale(locale);
    auto voices = m_tts->availableVoices();
    if(voices.size() == 0)
        return;
    for(auto& voice : voices)
    {
        QString text = QString("%1(%2)").arg(voice.name()).arg(voice.gender());
        ui->cboVoice->addItem(text);
    }
    ui->cboVoice->setCurrentIndex(0);
    m_tts->setVoice(voices.first());
}

void MainWindow::setTTsRepeat(bool repeat)
{
    m_ttsRepeat = repeat;
}

void MainWindow::onTTSStateChanged(QTextToSpeech::State state)
{
    if(     m_ttsRepeat &&
            state == QTextToSpeech::Ready &&
            m_presenter->isRunning() &&
            m_presenter->isOverTimeout())
    {
        auto ttsText = m_presenter->ttsText().trimmed();
        if(ttsText.size() == 0)
            return;
        m_tts->say(ttsText);
    }
}

void MainWindow::onTTSVoiceChanged(const QVoice &)
{
    if(     m_ttsRepeat &&
            m_presenter->isRunning() &&
            m_presenter->isOverTimeout())
    {
        auto ttsText = m_presenter->ttsText().trimmed();
        if(ttsText.size() == 0)
            return;
        m_tts->say(ttsText);
    }
}
