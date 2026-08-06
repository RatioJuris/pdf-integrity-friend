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

    connect(
        networkManager,
        &QNetworkAccessManager::finished,
        this,
        &MainWindow::onReachabilityResult);
}

MainWindow::~MainWindow() = default;

void MainWindow::appendLog(const QString& message)
{
    if (!logViewer)
        return;

    QString timestamp =
        QDateTime::currentDateTime()
            .toString("[yyyy-MM-dd hh:mm:ss] ");

    logViewer->appendPlainText(
        timestamp + message);
}

void MainWindow::setStatus(
    const QString& text,
    const QString& color)
{
    statusLabel->setText(text);

    statusLabel->setStyleSheet(
        QString(
            "font-weight:bold;"
            "font-size:13px;"
            "color:%1;")
            .arg(color));
}

void MainWindow::initUI()
{
    setWindowTitle("PDF Integrity Friend");
    setMinimumSize(900, 650);

    QWidget* centralWidget =
        new QWidget(this);

    setCentralWidget(centralWidget);

    auto* mainLayout =
        new QVBoxLayout(centralWidget);

    mainLayout->setContentsMargins(
        15, 15, 15, 15);

    mainLayout->setSpacing(15);

    QLabel* title =
        new QLabel("PDF Integrity Friend");

    title->setAlignment(Qt::AlignCenter);

    title->setStyleSheet(
        "font-size:26px;"
        "font-weight:bold;"
        "color:#1565c0;");

    QLabel* subtitle =
        new QLabel(
            "Secure PDF Signing and RFC3161 Time Stamping");

    subtitle->setAlignment(Qt::AlignCenter);

    subtitle->setStyleSheet(
        "font-size:12px;"
        "color:gray;");

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    QGroupBox* tsaBox =
        new QGroupBox("Time Stamp Authority");

    auto* tsaLayout =
        new QHBoxLayout(tsaBox);

    tsaComboBox = new QComboBox();

    checkBtn =
        new QPushButton("Verify Provider");

    providerStatus =
        new QLabel("● Unknown");

    providerStatus->setStyleSheet(
        "font-weight:bold;"
        "color:gray;");

    tsaLayout->addWidget(
        new QLabel("Provider:"));

    tsaLayout->addWidget(
        tsaComboBox,
        1);

    tsaLayout->addWidget(checkBtn);
    tsaLayout->addWidget(providerStatus);

    mainLayout->addWidget(tsaBox);

    QGroupBox* pdfBox =
        new QGroupBox("PDF Document");

    auto* pdfLayout =
        new QHBoxLayout(pdfBox);

    pdfPathEdit = new QLineEdit();

    pdfPathEdit->setPlaceholderText(
        "Select a PDF file...");

    QPushButton* browseButton =
        new QPushButton("Browse");

    pdfLayout->addWidget(pdfPathEdit);
    pdfLayout->addWidget(browseButton);

    mainLayout->addWidget(pdfBox);

    QGroupBox* progressBox =
        new QGroupBox("Operation Status");

    auto* progressLayout =
        new QVBoxLayout(progressBox);

    statusLabel =
        new QLabel("Ready");

    statusLabel->setAlignment(
        Qt::AlignCenter);

    progressBar =
        new QProgressBar();

    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    progressLayout->addWidget(
        statusLabel);

    progressLayout->addWidget(
        progressBar);

    mainLayout->addWidget(progressBox);

    QGroupBox* logBox =
        new QGroupBox("Activity Log");

    auto* logLayout =
        new QVBoxLayout(logBox);

    logViewer =
        new QPlainTextEdit();

    logViewer->setReadOnly(true);

    logLayout->addWidget(logViewer);

    mainLayout->addWidget(logBox);

    auto* buttonLayout =
        new QHBoxLayout();

    QPushButton* reloadButton =
        new QPushButton("Reload Config");

    QPushButton* centerButton =
        new QPushButton("Center Window");

    signBtn =
        new QPushButton("Sign PDF");

    signBtn->setMinimumHeight(40);

    buttonLayout->addStretch();
    buttonLayout->addWidget(reloadButton);
    buttonLayout->addWidget(centerButton);
    buttonLayout->addWidget(signBtn);

    mainLayout->addLayout(buttonLayout);

    connect(
        browseButton,
        &QPushButton::clicked,
        this,
        &MainWindow::selectPDFFile);

    connect(
        checkBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::checkTSAReachability);

    connect(
        signBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::signPDF);

    connect(
        reloadButton,
        &QPushButton::clicked,
        this,
        &MainWindow::loadTSAConfig);

    connect(
        centerButton,
        &QPushButton::clicked,
        this,
        &MainWindow::autoAdjustGeometry);
}

void MainWindow::autoAdjustGeometry()
{
    QScreen* screen =
        QGuiApplication::primaryScreen();

    if (!screen)
        return;

    QRect area =
        screen->availableGeometry();

    resize(900, 650);

    move(
        area.center().x() - width() / 2,
        area.center().y() - height() / 2);
}

bool MainWindow::validateJsonStructure(
    const QJsonObject& json)
{
    if (!json.contains("providers"))
        return false;

    if (!json["providers"].isArray())
        return false;

    QJsonArray providers =
        json["providers"].toArray();

    for (const auto& value : providers)
    {
        if (!value.isObject())
            return false;

        QJsonObject provider =
            value.toObject();

        if (!provider.contains("name"))
            return false;

        if (!provider.contains("url"))
            return false;
    }

    return true;
}

void MainWindow::createEmptyTemplateJson()
{
    QJsonObject root;

    root["providers"] =
        QJsonArray();

    QSaveFile file(
        configFilePath);

    if (!file.open(
        QIODevice::WriteOnly))
    {
        return;
    }

    QJsonDocument doc(root);

    file.write(
        doc.toJson(
            QJsonDocument::Indented));

    file.commit();
}

void MainWindow::loadTSAConfig()
{
    tsaComboBox->clear();

    QFile file(configFilePath);

    if (!file.exists())
    {
        createEmptyTemplateJson();

        setStatus(
            "Configuration file created.",
            "#ef6c00");

        appendLog(
            "Created empty TSA configuration.");

        return;
    }

    if (!file.open(
        QIODevice::ReadOnly |
        QIODevice::Text))
    {
        setStatus(
            "Unable to read configuration.",
            "#c62828");

        return;
    }

    QJsonParseError error;

    QJsonDocument doc =
        QJsonDocument::fromJson(
            file.readAll(),
            &error);

    file.close();

    if (error.error !=
            QJsonParseError::NoError ||
        !doc.isObject())
    {
        setStatus(
            "Invalid JSON configuration.",
            "#c62828");

        return;
    }

    QJsonObject root =
        doc.object();

    if (!validateJsonStructure(root))
    {
        setStatus(
            "Configuration validation failed.",
            "#c62828");

        return;
    }

    currentTSAData = root;

    QJsonArray providers =
        root["providers"].toArray();

    for (const auto& value : providers)
    {
        QJsonObject provider =
            value.toObject();

        tsaComboBox->addItem(
            provider["name"].toString(),
            provider["url"].toString());
    }

    appendLog(
        QString(
            "Loaded %1 TSA provider(s).")
            .arg(providers.size()));

    setStatus(
        "Configuration loaded successfully.",
        "#2e7d32");
}

void MainWindow::checkTSAReachability()
{
    if (tsaComboBox->count() == 0)
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "No TSA providers configured.");

        return;
    }

    checkBtn->setEnabled(false);

    progressBar->setRange(0, 0);

    appendLog(
        "Checking TSA provider...");

    setStatus(
        "Connecting...",
        "#1565c0");

    QString endpoint =
        tsaComboBox
            ->currentData()
            .toString();

    QNetworkRequest request(
        QUrl(endpoint));

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/timestamp-query");

    networkManager->post(
        request,
        QByteArray());
}

void MainWindow::onReachabilityResult(
    QNetworkReply *reply)
{
    checkBtn->setEnabled(true);

    progressBar->setRange(0, 100);
    progressBar->setValue(100);

    int statusCode =
        reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute)
            .toInt();

    bool success =
        reply->error() ==
            QNetworkReply::NoError ||
        statusCode == 200 ||
        statusCode == 400;

    if (success)
    {
        providerStatus->setText(
            "● Online");

        providerStatus->setStyleSheet(
            "color:#2e7d32;"
            "font-weight:bold;");

        setStatus(
            "Provider reachable.",
            "#2e7d32");

        appendLog(
            "Provider validation successful.");
    }
    else
    {
        providerStatus->setText(
            "● Offline");

        providerStatus->setStyleSheet(
            "color:#c62828;"
            "font-weight:bold;");

        setStatus(
            "Connection failed.",
            "#c62828");

        appendLog(
            reply->errorString());
    }

    reply->deleteLater();
}

void MainWindow::selectPDFFile()
{
    QString file =
        QFileDialog::getOpenFileName(
            this,
            "Select PDF Document",
            QString(),
            "PDF Files (*.pdf)");

    if (!file.isEmpty())
    {
        pdfPathEdit->setText(file);

        appendLog(
            QString("Selected PDF: %1")
            .arg(file));
    }
}

void MainWindow::signPDF()
{
    QString pdfFile =
        pdfPathEdit->text().trimmed();

    if (pdfFile.isEmpty())
    {
        QMessageBox::warning(
            this,
            "No PDF Selected",
            "Please select a PDF file.");

        return;
    }

    progressBar->setValue(0);

    appendLog(
        QString(
            "Preparing signing operation for %1")
            .arg(pdfFile));

    setStatus(
        "Preparing signing workflow...",
        "#1565c0");

    QMessageBox::information(
        this,
        "Coming Soon",
        "OpenSSL-based PDF signing and "
        "RFC3161 timestamping will be "
        "implemented in a future release.");
}
