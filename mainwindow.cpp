#include "mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QScreen>
#include <QUrl>
#include <QVBoxLayout>

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
        &MainWindow::onReachabilityResult
    );
}

MainWindow::~MainWindow() = default;

void MainWindow::initUI()
{
    setWindowTitle("PDF Integrity Friend");

    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // TSA Selection
    auto *tsaLayout = new QHBoxLayout();

    auto *tsaLabel = new QLabel("TSA Provider:");
    tsaComboBox = new QComboBox();

    tsaLayout->addWidget(tsaLabel);
    tsaLayout->addWidget(tsaComboBox);

    mainLayout->addLayout(tsaLayout);

    // PDF Selection
    auto *fileLayout = new QHBoxLayout();

    pdfPathEdit = new QLineEdit();
    pdfPathEdit->setPlaceholderText("Select a PDF document...");

    auto *browseButton = new QPushButton("Browse");

    connect(
        browseButton,
        &QPushButton::clicked,
        this,
        &MainWindow::selectPDFFile
    );

    fileLayout->addWidget(pdfPathEdit);
    fileLayout->addWidget(browseButton);

    mainLayout->addLayout(fileLayout);

    // Status
    statusLabel = new QLabel("Ready");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);

    mainLayout->addWidget(statusLabel);

    // Controls
    auto *buttonLayout = new QHBoxLayout();

    checkBtn = new QPushButton("Verify TSA");
    signBtn = new QPushButton("Sign PDF");

    auto *recenterButton = new QPushButton("Center Window");

    connect(
        checkBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::checkTSAReachability
    );

    connect(
        signBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::signPDF
    );

    connect(
        recenterButton,
        &QPushButton::clicked,
        this,
        &MainWindow::autoAdjustGeometry
    );

    buttonLayout->addWidget(checkBtn);
    buttonLayout->addWidget(signBtn);
    buttonLayout->addWidget(recenterButton);

    mainLayout->addLayout(buttonLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::autoAdjustGeometry()
{
    QScreen *screen = QGuiApplication::primaryScreen();

    if (!screen)
    {
        resize(600, 350);
        return;
    }

    const QRect geometry = screen->availableGeometry();

    const int width =
        qMax(600, static_cast<int>(geometry.width() * 0.30));

    const int height =
        qMax(350, static_cast<int>(geometry.height() * 0.25));

    resize(width, height);

    move(
        geometry.center().x() - width / 2,
        geometry.center().y() - height / 2
    );
}

void MainWindow::loadTSAConfig()
{
    QFile file(configFilePath);

    if (!file.exists())
    {
        createEmptyTemplateJson();

        statusLabel->setText(
            "Configuration file created. Add TSA providers and restart."
        );

        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        statusLabel->setText(
            "Unable to read TSA configuration."
        );

        return;
    }

    QJsonParseError error;
    QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &error);

    file.close();

    if (error.error != QJsonParseError::NoError ||
        !document.isObject())
    {
        statusLabel->setText(
            "Invalid configuration file."
        );

        return;
    }

    const QJsonObject root = document.object();

    if (!validateJsonStructure(root))
    {
        statusLabel->setText(
            "Configuration validation failed."
        );

        return;
    }

    currentTSAData = root;

    tsaComboBox->clear();

    const QJsonArray providers =
        root["providers"].toArray();

    for (const auto &providerValue : providers)
    {
        const QJsonObject provider =
            providerValue.toObject();

        tsaComboBox->addItem(
            provider["name"].toString(),
            provider["url"].toString()
        );
    }

    statusLabel->setText(
        QString("Loaded %1 TSA provider(s).")
            .arg(tsaComboBox->count())
    );
}

bool MainWindow::validateJsonStructure(
    const QJsonObject &json)
{
    if (!json.contains("providers") ||
        !json["providers"].isArray())
    {
        return false;
    }

    const QJsonArray providers =
        json["providers"].toArray();

    for (const auto &entry : providers)
    {
        if (!entry.isObject())
        {
            return false;
        }

        const QJsonObject provider =
            entry.toObject();

        if (!provider.contains("name") ||
            !provider.contains("url") ||
            !provider.contains("auth_required"))
        {
            return false;
        }

        if (provider["name"].toString().trimmed().isEmpty())
        {
            return false;
        }

        if (provider["url"].toString().trimmed().isEmpty())
        {
            return false;
        }
    }

    return true;
}

void MainWindow::createEmptyTemplateJson()
{
    QJsonObject root;
    root["providers"] = QJsonArray();

    QSaveFile file(configFilePath);

    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    QJsonDocument document(root);

    file.write(
        document.toJson(QJsonDocument::Indented)
    );

    file.commit();
}

void MainWindow::checkTSAReachability()
{
    if (tsaComboBox->count() == 0)
    {
        QMessageBox::warning(
            this,
            "No Providers",
            "No TSA providers are configured."
        );

        return;
    }

    const QString tsaUrl =
        tsaComboBox->currentData().toString();

    statusLabel->setText(
        "Checking TSA endpoint..."
    );

    checkBtn->setEnabled(false);

    QNetworkRequest request(QUrl(tsaUrl));

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/timestamp-query"
    );

    networkManager->post(request, QByteArray());
}

void MainWindow::onReachabilityResult(
    QNetworkReply *reply)
{
    checkBtn->setEnabled(true);

    const int statusCode =
        reply->attribute(
                 QNetworkRequest::HttpStatusCodeAttribute)
            .toInt();

    if (reply->error() == QNetworkReply::NoError ||
        statusCode == 200 ||
        statusCode == 400)
    {
        statusLabel->setText(
            "TSA endpoint is reachable."
        );

        statusLabel->setStyleSheet(
            "color: green; font-weight: bold;"
        );
    }
    else
    {
        statusLabel->setText(
            QString("Connection failed: %1")
                .arg(reply->errorString())
        );

        statusLabel->setStyleSheet(
            "color: red;"
        );
    }

    reply->deleteLater();
}

void MainWindow::selectPDFFile()
{
    const QString filePath =
        QFileDialog::getOpenFileName(
            this,
            "Select PDF Document",
            QString(),
            "PDF Files (*.pdf)"
        );

    if (!filePath.isEmpty())
    {
        pdfPathEdit->setText(filePath);
    }
}

void MainWindow::signPDF()
{
    const QString pdfFile =
        pdfPathEdit->text().trimmed();

    if (pdfFile.isEmpty())
    {
        QMessageBox::warning(
            this,
            "No PDF Selected",
            "Please select a PDF document first."
        );

        return;
    }

    QMessageBox::information(
        this,
        "Not Yet Implemented",
        "PDF signing functionality has not yet been implemented."
    );
}
