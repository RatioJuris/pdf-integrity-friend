#include "mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QSaveFile>
#include <QScreen>
#include <QToolButton>
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
    setMinimumSize(850, 520);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ===================================================
    // Header
    // ===================================================

    QLabel *titleLabel =
        new QLabel("PDF Integrity Friend");

    titleLabel->setAlignment(Qt::AlignCenter);

    titleLabel->setStyleSheet(
        "font-size:24px;"
        "font-weight:bold;"
        "color:#1565c0;"
    );

    QLabel *subTitle =
        new QLabel(
            "PDF Signing and RFC3161 Time Stamping"
        );

    subTitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subTitle);

    // ===================================================
    // TSA Configuration
    // ===================================================

    QGroupBox *tsaGroup =
        new QGroupBox("Time Stamp Authority");

    auto *tsaLayout =
        new QHBoxLayout(tsaGroup);

    tsaComboBox = new QComboBox();

    checkBtn =
        new QPushButton("Verify Provider");

    tsaLayout->addWidget(
        new QLabel("Provider:")
    );

    tsaLayout->addWidget(
        tsaComboBox,
        1
    );

    tsaLayout->addWidget(
        checkBtn
    );

    mainLayout->addWidget(tsaGroup);

    // ===================================================
    // PDF Selection
    // ===================================================

    QGroupBox *pdfGroup =
        new QGroupBox("Document");

    auto *pdfLayout =
        new QHBoxLayout(pdfGroup);

    pdfPathEdit = new QLineEdit();

    pdfPathEdit->setPlaceholderText(
        "Select a PDF document..."
    );

    QPushButton *browseButton =
        new QPushButton("Browse");

    pdfLayout->addWidget(
        new QLabel("PDF:")
    );

    pdfLayout->addWidget(
        pdfPathEdit,
        1
    );

    pdfLayout->addWidget(
        browseButton
    );

    mainLayout->addWidget(pdfGroup);

    // ===================================================
    // Status Section
    // ===================================================

    QGroupBox *statusGroup =
        new QGroupBox("Status");

    auto *statusLayout =
        new QVBoxLayout(statusGroup);

    statusLabel = new QLabel("Ready");

    statusLabel->setAlignment(
        Qt::AlignCenter
    );

    statusLabel->setStyleSheet(
        "font-weight:bold;"
    );

    progressBar = new QProgressBar();

    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(0);

    statusLayout->addWidget(
        statusLabel
    );

    statusLayout->addWidget(
        progressBar
    );

    mainLayout->addWidget(statusGroup);

    // ===================================================
    // Action Buttons
    // ===================================================

    auto *actionLayout =
        new QHBoxLayout();

    QPushButton *reloadBtn =
        new QPushButton("Reload Config");

    QPushButton *centerBtn =
        new QPushButton("Center Window");

    signBtn =
        new QPushButton("Sign PDF");

    signBtn->setMinimumHeight(40);

    actionLayout->addStretch();

    actionLayout->addWidget(reloadBtn);
    actionLayout->addWidget(centerBtn);
    actionLayout->addWidget(signBtn);

    mainLayout->addLayout(actionLayout);

    // ===================================================
    // Footer
    // ===================================================

    QLabel *footer =
        new QLabel(
            "© RatioJuris"
        );

    footer->setAlignment(
        Qt::AlignCenter
    );

    footer->setStyleSheet(
        "color:gray;"
    );

    mainLayout->addWidget(footer);

    // ===================================================
    // Connections
    // ===================================================

    connect(
        browseButton,
        &QPushButton::clicked,
        this,
        &MainWindow::selectPDFFile
    );

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
        reloadBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::loadTSAConfig
    );

    connect(
        centerBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::autoAdjustGeometry
    );
}

void MainWindow::autoAdjustGeometry()
{
    auto *screen =
        QGuiApplication::primaryScreen();

    if (!screen)
        return;

    QRect area =
        screen->availableGeometry();

    resize(850, 520);

    move(
        area.center().x() - width() / 2,
        area.center().y() - height() / 2
    );
}

void MainWindow::checkTSAReachability()
{
    if (tsaComboBox->count() == 0)
    {
        QMessageBox::warning(
            this,
            "Configuration",
            "No TSA providers configured."
        );

        return;
    }

    progressBar->setValue(25);

    statusLabel->setText(
        "Connecting to TSA..."
    );

    checkBtn->setEnabled(false);

    QString url =
        tsaComboBox->currentData().toString();

    QNetworkRequest request(
        QUrl(url)
    );

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/timestamp-query"
    );

    networkManager->post(
        request,
        QByteArray()
    );
}

void MainWindow::onReachabilityResult(
    QNetworkReply *reply)
{
    checkBtn->setEnabled(true);

    int statusCode =
        reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute
        ).toInt();

    progressBar->setValue(100);

    if (reply->error() ==
            QNetworkReply::NoError ||
        statusCode == 200 ||
        statusCode == 400)
    {
        statusLabel->setText(
            "Provider reachable"
        );

        statusLabel->setStyleSheet(
            "color:#2e7d32;"
            "font-weight:bold;"
        );
    }
    else
    {
        statusLabel->setText(
            "Connection failed"
        );

        statusLabel->setStyleSheet(
            "color:#c62828;"
            "font-weight:bold;"
        );
    }

    reply->deleteLater();
}

void MainWindow::signPDF()
{
    if (pdfPathEdit->text().isEmpty())
    {
        QMessageBox::warning(
            this,
            "No PDF",
            "Select a PDF file first."
        );

        return;
    }

    progressBar->setValue(10);
    statusLabel->setText(
        "Preparing signing process..."
    );

    QApplication::processEvents();

    progressBar->setValue(40);
    QApplication::processEvents();

    progressBar->setValue(70);
    QApplication::processEvents();

    progressBar->setValue(100);

    statusLabel->setText(
        "Signing feature coming soon."
    );

    QMessageBox::information(
        this,
        "Feature Pending",
        "RFC3161 timestamping and PDF signing "
        "are not implemented yet."
    );
}
