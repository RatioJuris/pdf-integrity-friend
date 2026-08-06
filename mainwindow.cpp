#include "mainwindow.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      networkManager(new QNetworkAccessManager(this)),
      configFilePath("tsa_config.json")
{
    initUI();
    loadTSAConfig();
    autoAdjustGeometry();

    connect(networkManager,
            &QNetworkAccessManager::finished,
            this,
            &MainWindow::onReachabilityResult);
}

MainWindow::~MainWindow() = default;

void MainWindow::appendLog(const QString &message)
{
    if (!logViewer)
        return;

    logViewer->appendPlainText(
        QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ") +
        message);
}

void MainWindow::setStatus(const QString &text, const QString &color)
{
    statusLabel->setText(text);
    statusLabel->setStyleSheet(QString("font-weight:bold;color:%1;").arg(color));
}

void MainWindow::initUI()
{
    setWindowTitle("PDF Integrity Friend");
    setMinimumSize(900, 650);

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    auto *title = new QLabel("PDF Integrity Friend");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:26px;font-weight:bold;color:#1565c0;");

    auto *subtitle = new QLabel("Secure PDF Signing and RFC3161 Time Stamping");
    subtitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto *tsaBox = new QGroupBox("Time Stamp Authority");
    auto *tsaLayout = new QHBoxLayout(tsaBox);

    tsaComboBox = new QComboBox();
    checkBtn = new QPushButton("Verify Provider");
    providerStatus = new QLabel("● Unknown");

    tsaLayout->addWidget(new QLabel("Provider:"));
    tsaLayout->addWidget(tsaComboBox, 1);
    tsaLayout->addWidget(checkBtn);
    tsaLayout->addWidget(providerStatus);

    mainLayout->addWidget(tsaBox);

    auto *pdfBox = new QGroupBox("PDF Document");
    auto *pdfLayout = new QHBoxLayout(pdfBox);

    pdfPathEdit = new QLineEdit();
    auto *browseButton = new QPushButton("Browse");

    pdfLayout->addWidget(pdfPathEdit);
    pdfLayout->addWidget(browseButton);

    mainLayout->addWidget(pdfBox);

    auto *statusBox = new QGroupBox("Operation Status");
    auto *statusLayout = new QVBoxLayout(statusBox);

    statusLabel = new QLabel("Ready");
    progressBar = new QProgressBar();

    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(progressBar);

    mainLayout->addWidget(statusBox);

    auto *logBox = new QGroupBox("Activity Log");
    auto *logLayout = new QVBoxLayout(logBox);

    logViewer = new QPlainTextEdit();
    logViewer->setReadOnly(true);

    logLayout->addWidget(logViewer);
    mainLayout->addWidget(logBox, 1);

    auto *buttons = new QHBoxLayout();
    auto *reloadButton = new QPushButton("Reload Config");
    auto *centerButton = new QPushButton("Center Window");
    signBtn = new QPushButton("Sign PDF");

    buttons->addStretch();
    buttons->addWidget(reloadButton);
    buttons->addWidget(centerButton);
    buttons->addWidget(signBtn);

    mainLayout->addLayout(buttons);

    connect(browseButton,&QPushButton::clicked,this,&MainWindow::selectPDFFile);
    connect(checkBtn,&QPushButton::clicked,this,&MainWindow::checkTSAReachability);
    connect(signBtn,&QPushButton::clicked,this,&MainWindow::signPDF);
    connect(reloadButton,&QPushButton::clicked,this,&MainWindow::loadTSAConfig);
    connect(centerButton,&QPushButton::clicked,this,&MainWindow::autoAdjustGeometry);
}

void MainWindow::autoAdjustGeometry()
{
    auto *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect area = screen->availableGeometry();
    move(area.center().x() - width()/2, area.center().y() - height()/2);
}

bool MainWindow::validateJsonStructure(const QJsonObject &json)
{
    return json.contains("providers") && json["providers"].isArray();
}

void MainWindow::createEmptyTemplateJson()
{
    QJsonObject root;
    root["providers"] = QJsonArray();

    QSaveFile file(configFilePath);
    if (!file.open(QIODevice::WriteOnly)) return;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.commit();
}

void MainWindow::loadTSAConfig()
{
    tsaComboBox->clear();

    QFile file(configFilePath);

    if (!file.exists())
    {
        createEmptyTemplateJson();
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject())
        return;

    QJsonArray providers = doc.object()["providers"].toArray();

    for (const auto &value : providers)
    {
        QJsonObject provider = value.toObject();
        tsaComboBox->addItem(provider["name"].toString(), provider["url"].toString());
    }
}

void MainWindow::checkTSAReachability()
{
    if (tsaComboBox->count() == 0)
        return;

    progressBar->setRange(0,0);

    QNetworkRequest request(QUrl(tsaComboBox->currentData().toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/timestamp-query");

    networkManager->post(request, QByteArray());
}

void MainWindow::onReachabilityResult(QNetworkReply *reply)
{
    progressBar->setRange(0,100);
    progressBar->setValue(100);

    if (reply->error() == QNetworkReply::NoError)
    {
        providerStatus->setText("● Online");
        setStatus("Provider reachable", "#2e7d32");
    }
    else
    {
        providerStatus->setText("● Offline");
        setStatus(reply->errorString(), "#c62828");
    }

    reply->deleteLater();
}

void MainWindow::selectPDFFile()
{
    QString file = QFileDialog::getOpenFileName(
        this, "Select PDF", QString(), "PDF Files (*.pdf)");

    if (!file.isEmpty())
        pdfPathEdit->setText(file);
}

void MainWindow::signPDF()
{
    QMessageBox::information(
        this,
        "Coming Soon",
        "PDF signing functionality is not implemented yet.");
}
