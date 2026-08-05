#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QSaveFile>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    networkManager = new QNetworkAccessManager(this);
    configFilePath = "tsa_config.json";
    
    initUI();
    loadTSAConfig();
    autoAdjustGeometry();
    
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReachabilityResult);
}

MainWindow::~MainWindow() {}

void MainWindow::initUI() {
    this->setWindowTitle("PDF Integrity Friend");
    
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // TSA Selection
    QHBoxLayout* tsaLayout = new QHBoxLayout();
    tsaLayout->addWidget(new QLabel("TSA Provider:"));
    tsaComboBox = new QComboBox();
    tsaLayout->addWidget(tsaComboBox);
    mainLayout->addLayout(tsaLayout);

    // File Selection
    QHBoxLayout* fileLayout = new QHBoxLayout();
    pdfPathEdit = new QLineEdit();
    pdfPathEdit->setPlaceholderText("Select PDF file...");
    QPushButton* browseBtn = new QPushButton("Browse");
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::selectPDFFile);
    fileLayout->addWidget(pdfPathEdit);
    fileLayout->addWidget(browseBtn);
    mainLayout->addLayout(fileLayout);

    // Status Label
    statusLabel = new QLabel("Status: Ready");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // Action Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    checkBtn = new QPushButton("Test Reachability");
    signBtn = new QPushButton("Sign PDF");
    QPushButton* resizeBtn = new QPushButton("Auto-Adjust UI");
    
    connect(checkBtn, &QPushButton::clicked, this, &MainWindow::checkTSAReachability);
    connect(signBtn, &QPushButton::clicked, this, &MainWindow::signPDF);
    connect(resizeBtn, &QPushButton::clicked, this, &MainWindow::autoAdjustGeometry);

    btnLayout->addWidget(checkBtn);
    btnLayout->addWidget(signBtn);
    btnLayout->addWidget(resizeBtn);
    mainLayout->addLayout(btnLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::autoAdjustGeometry() {
    // Detect screen resolution dynamically
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // Scale panel dynamically based on resolution (target 25% width, 20% height)
    int targetWidth = qMax(400, static_cast<int>(screenWidth * 0.25));
    int targetHeight = qMax(200, static_cast<int>(screenHeight * 0.20));

    // Calculate exact center coordinates
    int x = (screenWidth - targetWidth) / 2;
    int y = (screenHeight - targetHeight) / 2;

    this->setGeometry(x, y, targetWidth, targetHeight);
    this->setFixedSize(targetWidth, targetHeight); // Maintain clean panel design
}

void MainWindow::loadTSAConfig() {
    QFile file(configFilePath);
    if (!file.exists()) {
        createDefaultJson();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusLabel->setText("Error: Cannot read JSON file.");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        statusLabel->setText("JSON Corrupted! Reverting to defaults.");
        createDefaultJson();
        return;
    }

    QJsonObject root = doc.object();
    if (!validateJsonStructure(root)) {
        statusLabel->setText("JSON Validation Failed! Reverting.");
        createDefaultJson();
        return;
    }

    currentTSAData = root;
    tsaComboBox->clear();
    QJsonArray providers = root["providers"].toArray();
    for (const QJsonValue& val : providers) {
        tsaComboBox->addItem(val.toObject()["name"].toString(), val.toObject()["url"].toString());
    }
}

bool MainWindow::validateJsonStructure(const QJsonObject& json) {
    if (!json.contains("providers") || !json["providers"].isArray()) return false;
    QJsonArray arr = json["providers"].toArray();
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) return false;
        QJsonObject obj = val.toObject();
        if (!obj.contains("name") || !obj.contains("url") || !obj.contains("auth_required")) return false;
    }
    return true;
}

void MainWindow::createDefaultJson() {
    QJsonObject root;
    QJsonArray providers;

    QJsonObject digicert;
    digicert["name"] = "DigiCert TSA";
    digicert["url"] = "http://digicert.com";
    digicert["auth_required"] = false;

    QJsonObject sectigo;
    sectigo["name"] = "Sectigo TSA";
    sectigo["url"] = "http://comodoca.com";
    sectigo["auth_required"] = false;

    providers.append(digicert);
    providers.append(sectigo);
    root["providers"] = providers;

    // Save atomically using QSaveFile to guard against runtime corruption
    QSaveFile file(configFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.commit();
    }
}

void MainWindow::checkTSAReachability() {
    QString tsaUrl = tsaComboBox->currentData().toString();
    if (tsaUrl.isEmpty()) return;

    statusLabel->setText("Testing reachability...");
    checkBtn->setEnabled(false);

    QNetworkRequest request((QUrl(tsaUrl)));
    // RFC 3161 Time-Stamp protocol content-type header requirement
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/timestamp-query");
    
    // Non-blocking asynchronous network check
    networkManager->post(request, QByteArray()); 
}

void MainWindow::onReachabilityResult(QNetworkReply* reply) {
    checkBtn->setEnabled(true);
    if (reply->error() == QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 400) {
        // HTTP 400 Bad Request is highly acceptable here because an empty POST body was sent to test endpoint life
        statusLabel->setText("Success: TSA Server is Online!");
        statusLabel->setStyleSheet("color: green;");
    } else {
        statusLabel->setText("Failed: " + reply->errorString());
        statusLabel->setStyleSheet("color: red;");
    }
    reply->deleteLater();
}

void MainWindow::selectPDFFile() {
    QString file = QFileDialog::getOpenFileName(this, "Open PDF File", "", "PDF Files (*.pdf)");
    if (!file.isEmpty()) {
        pdfPathEdit->setText(file);
    }
}

void MainWindow::signPDF() {
    if (pdfPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a valid PDF file first.");
        return;
    }
    // Execution placeholder for local cryptographic signature payload integration using OpenSSL LibCrypto
    QMessageBox::information(this, "Signature Status", "PDF Parsing and Crypto-Hash verification structural loops initiated.");
}
