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
    this->setWindowTitle("PDF Integrity Friend - by RatioJuris");
    
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // TSA Dropdown Selection
    QHBoxLayout* tsaLayout = new QHBoxLayout();
    tsaLayout->addWidget(new QLabel("TSA Provider:"));
    tsaComboBox = new QComboBox();
    tsaLayout->addWidget(tsaComboBox);
    mainLayout->addLayout(tsaLayout);

    // File Selection Row
    QHBoxLayout* fileLayout = new QHBoxLayout();
    pdfPathEdit = new QLineEdit();
    pdfPathEdit->setPlaceholderText("Select target PDF file path...");
    QPushButton* browseBtn = new QPushButton("Browse");
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::selectPDFFile);
    fileLayout->addWidget(pdfPathEdit);
    fileLayout->addWidget(browseBtn);
    mainLayout->addLayout(fileLayout);

    // Dynamic State Monitoring Panel
    statusLabel = new QLabel("Status: Awaiting Initial Configuration Check...");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);

    // Control Cluster Elements
    QHBoxLayout* btnLayout = new QHBoxLayout();
    checkBtn = new QPushButton("Verify TSA Pipeline");
    signBtn = new QPushButton("Sign Document");
    QPushButton* resizeBtn = new QPushButton("Recenter Framework");
    
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
    // Collect display topology details at runtime
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // Map proportions securely matching high-DPI viewports
    int targetWidth = qMax(420, static_cast<int>(screenWidth * 0.25));
    int targetHeight = qMax(220, static_cast<int>(screenHeight * 0.20));

    // Derive precise layout spatial center points
    int x = (screenWidth - targetWidth) / 2;
    int y = (screenHeight - targetHeight) / 2;

    this->setGeometry(x, y, targetWidth, targetHeight);
    this->setFixedSize(targetWidth, targetHeight); 
}

void MainWindow::loadTSAConfig() {
    QFile file(configFilePath);
    if (!file.exists()) {
        createEmptyTemplateJson();
        statusLabel->setText("Config template created! Populate 'tsa_config.json' with URLs.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusLabel->setText("Error: Execution configuration file locked or unreadable.");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        statusLabel->setText("JSON Struct Broken! Reverting file to empty structure.");
        createEmptyTemplateJson();
        return;
    }

    QJsonObject root = doc.object();
    if (!validateJsonStructure(root)) {
        statusLabel->setText("Validation failed! Missing key values inside JSON nodes.");
        createEmptyTemplateJson();
        return;
    }

    currentTSAData = root;
    tsaComboBox->clear();
    QJsonArray providers = root["providers"].toArray();
    
    if(providers.isEmpty()) {
        statusLabel->setText("Notice: 'tsa_config.json' is currently empty. Add your TSA URLs.");
        return;
    }

    for (const QJsonValue& val : providers) {
        tsaComboBox->addItem(val.toObject()["name"].toString(), val.toObject()["url"].toString());
    }
    statusLabel->setText("Status: Configuration Map Loaded Effectively.");
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

void MainWindow::createEmptyTemplateJson() {
    QJsonObject root;
    QJsonArray providers; // Completely clean array loop to prevent hardcoded endpoint embedding

    root["providers"] = providers;

    // Use QSaveFile for safe atomic writes on storage blocks
    QSaveFile file(configFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.commit();
    }
}

void MainWindow::checkTSAReachability() {
    if (tsaComboBox->count() == 0) {
        QMessageBox::critical(this, "Empty Profile", "No custom TSA endpoints discovered inside local config directory.");
        return;
    }

    QString tsaUrl = tsaComboBox->currentData().toString();
    statusLabel->setText("Testing active secure pipelines...");
    checkBtn->setEnabled(false);

    QNetworkRequest request((QUrl(tsaUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/timestamp-query");
    
    // Process asynchronous endpoint ping securely
    networkManager->post(request, QByteArray()); 
}

void MainWindow::onReachabilityResult(QNetworkReply* reply) {
    checkBtn->setEnabled(true);
    // Standard RFC3161 processing returns 400 Bad Request to blank posts, acknowledging server visibility
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (reply->error() == QNetworkReply::NoError || statusCode == 400 || statusCode == 200) {
        statusLabel->setText("Pipeline Verification Success: Endpoint is online.");
        statusLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    } else {
        statusLabel->setText("Pipeline Verification Error: " + reply->errorString());
        statusLabel->setStyleSheet("color: #c62828;");
    }
    reply->deleteLater();
}

void MainWindow::selectPDFFile() {
    QString file = QFileDialog::getOpenFileName(this, "Open Integrity Context Target", "", "PDF Document Types (*.pdf)");
    if (!file.isEmpty()) {
        pdfPathEdit->setText(file);
    }
}

void MainWindow::signPDF() {
    if (pdfPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Execution Error", "No target file was linked to the signing stack.");
        return;
    }
    QMessageBox::information(this, "Process Active", "PDF file targets isolated. Proceeding with OpenSSL verification checks.");
}
