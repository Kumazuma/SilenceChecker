#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QListWidget>
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
      m_presenter(std::move(presenter))

{
    ui->setupUi(this);

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

    connect(m_presenter.get(), &Presenter::tickTime,[this](int secs)->void
    {
        int sec = secs % 60;
        int min = (secs / 60)%60;
        int hour = secs / 360;
        ui->lcdSec->display(sec);
        ui->lcdMin->display(min);
        ui->lcdHour->display(hour);
    });
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),m_presenter.get(), SLOT(onInputDeviceChanged(int)));

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
