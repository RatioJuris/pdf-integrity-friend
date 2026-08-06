#include "MainWindow.h"

#include <QAction>

#include <QApplication>

#include <QCheckBox>

#include <QComboBox>

#include <QFile>

#include <QFileDialog>

#include <QGroupBox>

#include <QHBoxLayout>

#include <QJsonArray>

#include <QJsonDocument>

#include <QJsonObject>

#include <QLabel>

#include <QListWidget>

#include <QMenu>

#include <QMenuBar>

#include <QMessageBox>

#include <QNetworkAccessManager>

#include <QNetworkReply>

#include <QNetworkRequest>

#include <QPdfDocument>

#include <QPdfView>

#include <QPlainTextEdit>

#include <QProgressBar>

#include <QPushButton>

#include <QSaveFile>

#include <QSplitter>

#include <QStatusBar>

#include <QTime>

#include <QVBoxLayout>

#include <QWidget>

MainWindow::MainWindow(QWidget * parent): QMainWindow(parent),
  pdfDocument(new QPdfDocument(this)),
  networkManager(new QNetworkAccessManager(this)),
  configFilePath(QStringLiteral("tsa_config.json")) {
    try {
      initializeUi();
      initializeMenuBar();
      initializeStatusBar();
      loadTSAConfig();

      connect(networkManager, & QNetworkAccessManager::finished,
        this, & MainWindow::onReachabilityResult);

      appendLog(QStringLiteral("Application initialized successfully."));
    } catch (const std::exception & ex) {
      QMessageBox::critical(this, QStringLiteral("Initialization Error"), QString::fromUtf8(ex.what()));
    }
  }

MainWindow::~MainWindow() =
  default;

void MainWindow::initializeUi() {
  auto * centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  auto * mainLayout = new QVBoxLayout(centralWidget);
  mainSplitter = new QSplitter(Qt::Horizontal, this);
  mainLayout -> addWidget(mainSplitter);

  // LEFT SIDE
  auto * leftPanel = new QWidget(this);
  auto * leftLayout = new QVBoxLayout(leftPanel);
  leftLayout -> setContentsMargins(0, 0, 0, 0);

  auto * pagesBox = new QGroupBox(QStringLiteral("Pages"), this);
  auto * pagesLayout = new QVBoxLayout(pagesBox);
  pageList = new QListWidget(this);
  pagesLayout -> addWidget(pageList);
  leftLayout -> addWidget(pagesBox);

  auto * signatureBox = new QGroupBox(QStringLiteral("Signatures"), this);
  auto * signatureLayout = new QVBoxLayout(signatureBox);
  signatureList = new QListWidget(this);
  addSignatureButton = new QPushButton(QStringLiteral("Add Signature"), this);
  removeSignatureButton = new QPushButton(QStringLiteral("Remove Signature"), this);

  signatureLayout -> addWidget(signatureList);
  signatureLayout -> addWidget(addSignatureButton);
  signatureLayout -> addWidget(removeSignatureButton);
  leftLayout -> addWidget(signatureBox);

  // CENTER PANEL
  auto * centerPanel = new QWidget(this);
  auto * centerLayout = new QVBoxLayout(centerPanel);
  centerLayout -> setContentsMargins(0, 0, 0, 0);

  auto * pdfBox = new QGroupBox(QStringLiteral("PDF Viewer"), this);
  auto * pdfLayout = new QVBoxLayout(pdfBox);
  pdfView = new QPdfView(this);
  pdfView -> setDocument(pdfDocument);
  pdfLayout -> addWidget(pdfView);
  centerLayout -> addWidget(pdfBox);

  // RIGHT SIDE
  auto * rightPanel = new QWidget(this);
  auto * rightLayout = new QVBoxLayout(rightPanel);
  rightLayout -> setContentsMargins(0, 0, 0, 0);

  auto * tsaBox = new QGroupBox(QStringLiteral("TSA Configuration"), this);
  auto * tsaLayout = new QVBoxLayout(tsaBox);
  tsaComboBox = new QComboBox(this);
  checkTsaButton = new QPushButton(QStringLiteral("Check TSA"), this);

  auto * tsaStatusLayout = new QHBoxLayout();
  auto * tsaStatusTitle = new QLabel(QStringLiteral("Status:"), this);
  tsaStatusLabel = new QLabel(QStringLiteral("Unknown"), this);
  tsaStatusLayout -> addWidget(tsaStatusTitle);
  tsaStatusLayout -> addWidget(tsaStatusLabel);
  tsaStatusLayout -> addStretch();

  tsaLayout -> addWidget(tsaComboBox);
  tsaLayout -> addWidget(checkTsaButton);
  tsaLayout -> addLayout(tsaStatusLayout);
  rightLayout -> addWidget(tsaBox);

  auto * securityBox = new QGroupBox(QStringLiteral("PDF Permissions"), this);
  auto * securityLayout = new QVBoxLayout(securityBox);
  disableEditingCheckBox = new QCheckBox(QStringLiteral("Disable Editing"), this);
  disableCopyCheckBox = new QCheckBox(QStringLiteral("Disable Copy"), this);
  disablePrintCheckBox = new QCheckBox(QStringLiteral("Disable Print"), this);
  disableCommentCheckBox = new QCheckBox(QStringLiteral("Disable Comments"), this);

  securityLayout -> addWidget(disableEditingCheckBox);
  securityLayout -> addWidget(disableCopyCheckBox);
  securityLayout -> addWidget(disablePrintCheckBox);
  securityLayout -> addWidget(disableCommentCheckBox);
  rightLayout -> addWidget(securityBox);
  rightLayout -> addStretch();

  mainSplitter -> addWidget(leftPanel);
  mainSplitter -> addWidget(centerPanel);
  mainSplitter -> addWidget(rightPanel);
  mainSplitter -> setStretchFactor(0, 1);
  mainSplitter -> setStretchFactor(1, 3);
  mainSplitter -> setStretchFactor(2, 1);

  // LOGS
  auto * logBox = new QGroupBox(QStringLiteral("System Log Output"), this);
  auto * logLayout = new QVBoxLayout(logBox);
  logOutput = new QPlainTextEdit(this);
  logOutput -> setReadOnly(true);
  logLayout -> addWidget(logOutput);
  mainLayout -> addWidget(logBox);

  connect(addSignatureButton, & QPushButton::clicked, this, & MainWindow::addVisibleSignature);
  connect(removeSignatureButton, & QPushButton::clicked, this, & MainWindow::removeSelectedSignature);
  connect(checkTsaButton, & QPushButton::clicked, this, & MainWindow::checkTSAReachability);
}

void MainWindow::initializeMenuBar() {
  auto * fileMenu = menuBar() -> addMenu(QStringLiteral("&File"));
  fileMenu -> addAction(QStringLiteral("&Open PDF..."), this, & MainWindow::openPdf, QKeySequence::Open);
  fileMenu -> addAction(QStringLiteral("&Save Signed PDF..."), this, & MainWindow::savePdf, QKeySequence::Save);
  fileMenu -> addSeparator();
  fileMenu -> addAction(QStringLiteral("&Exit"), qApp, & QApplication::quit, QKeySequence::Quit);

  auto * viewMenu = menuBar() -> addMenu(QStringLiteral("&View"));
  viewMenu -> addAction(QStringLiteral("&Toggle Theme"), this, & MainWindow::toggleTheme, QKeySequence(Qt::CTRL | Qt::Key_T));
}

void MainWindow::initializeStatusBar() {
  statusLabel = new QLabel(QStringLiteral("Ready"), this);
  progressBar = new QProgressBar(this);
  progressBar -> setMaximum(100);
  progressBar -> setValue(0);
  progressBar -> setFixedWidth(150);

  statusBar() -> addWidget(statusLabel, 1);
  statusBar() -> addPermanentWidget(progressBar);
}

void MainWindow::openPdf() {
  const QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("Open Document"), QString(), QStringLiteral("PDF Files (*.pdf)"));
  if (filePath.isEmpty()) return;

  currentPdfFile = filePath;
  progressBar -> setValue(25);
  setStatus(QStringLiteral("Loading PDF file..."));

  pdfDocument -> load(filePath);
  progressBar -> setValue(100);

  if (pdfDocument -> error() == QPdfDocument::NoError) {
    appendLog(QStringLiteral("Loaded document successfully: %1 (%2 pages)").arg(filePath).arg(pdfDocument -> pageCount()));
    setStatus(QStringLiteral("Document loaded completely."));

    pageList -> clear();
    for (int i = 0; i < pdfDocument -> pageCount(); ++i) {
      pageList -> addItem(QStringLiteral("Page %1").arg(i + 1));
    }
  } else {
    appendLog(QStringLiteral("Failed to load PDF document structure."));
    setStatus(QStringLiteral("Error opening PDF."), QStringLiteral("#d32f2f"));
  }
}

void MainWindow::savePdf() {
  if (currentPdfFile.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("Save Aborted"), QStringLiteral("No primary active layout document source reference has been parsed."));
    return;
  }

  const QString savePath = QFileDialog::getSaveFileName(this, QStringLiteral("Export Secure Document"), QString(), QStringLiteral("PDF Files (*.pdf)"));
  if (savePath.isEmpty()) return;

  appendLog(QStringLiteral("Exporting document to: %1").arg(savePath));
  setStatus(QStringLiteral("Persisting secure changes..."));
}

void MainWindow::loadTSAConfig() {
  try {
    tsaComboBox -> clear();
    QFile file(configFilePath);

    if (!file.exists()) {
      createEmptyTemplateJson();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      appendLog(QStringLiteral("Unable to open TSA configuration file."));
      setStatus(QStringLiteral("Failed to open TSA configuration."), QStringLiteral("#d32f2f"));
      return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), & parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
      appendLog(QStringLiteral("JSON parse error: %1").arg(parseError.errorString()));
      setStatus(QStringLiteral("Invalid TSA configuration."), QStringLiteral("#d32f2f"));
      return;
    }

    if (!document.isObject()) {
      appendLog(QStringLiteral("Root JSON node is not an object."));
      return;
    }

    currentTSAData = document.object();

    if (!validateJsonStructure(currentTSAData)) {
      appendLog(QStringLiteral("TSA configuration validation failed."));
      return;
    }

    const QJsonArray providers = currentTSAData.value(QStringLiteral("providers")).toArray();

    for (const QJsonValue & value: providers) {
      const QJsonObject provider = value.toObject();
      tsaComboBox -> addItem(
        provider.value(QStringLiteral("name")).toString(),
        provider.value(QStringLiteral("url")).toString()
      );
    }

    appendLog(QStringLiteral("Loaded %1 TSA provider(s).").arg(providers.count()));
    setStatus(QStringLiteral("TSA configuration loaded successfully."), QStringLiteral("#2e7d32"));
  } catch (const std::exception & ex) {
    appendLog(QStringLiteral("Exception during TSA load: %1").arg(QString::fromUtf8(ex.what())));
    QMessageBox::critical(this, QStringLiteral("TSA Configuration"), QString::fromUtf8(ex.what()));

  } catch (...) {
    appendLog(QStringLiteral("Unknown exception during TSA load."));
    QMessageBox::critical(this, QStringLiteral("TSA Configuration"), QStringLiteral("An unknown error occurred."));

  }
}
void MainWindow::checkTSAReachability() {
  const QString targetUrl = tsaComboBox -> currentData().toString();
  if (targetUrl.isEmpty()) {
    appendLog(QStringLiteral("TSA lookup canceled: Parameter targeting validation error."));
    return;
  }
  setStatus(QStringLiteral("Querying remote TSA server status..."));
  tsaStatusLabel -> setText(QStringLiteral("Pinging..."));
  tsaStatusLabel -> setStyleSheet(QStringLiteral("color: #e65100; font-weight: bold;"));
  QNetworkRequest request {
    QUrl(targetUrl)
  };
  networkManager -> get(request);
}
void MainWindow::onReachabilityResult(QNetworkReply * reply) {
  if (!reply) return;
  reply -> deleteLater();
  if (reply -> error() == QNetworkReply::NoError) {
    tsaStatusLabel -> setText(QStringLiteral("Online"));
    tsaStatusLabel -> setStyleSheet(QStringLiteral("color: #2e7d32; font-weight: bold;"));
    appendLog(QStringLiteral("TSA network target handshake verified operational."));
    setStatus(QStringLiteral("Remote service target online."));
  } else {
    tsaStatusLabel -> setText(QStringLiteral("Offline"));
    tsaStatusLabel -> setStyleSheet(QStringLiteral("color: #c62828; font-weight: bold;"));
    appendLog(QStringLiteral("TSA network operational layer trace fault: %1").arg(reply -> errorString()));
    setStatus(QStringLiteral("Remote structural endpoint testing failure detected."), QStringLiteral("#d32f2f"));
  }
}
void MainWindow::addVisibleSignature() {
  signatureList -> addItem(QStringLiteral("Signature Component #%1").arg(signatureList -> count() + 1));
  appendLog(QStringLiteral("Appended dynamic relative structural mock contextual signature matrix marker layer."));
}
void MainWindow::removeSelectedSignature() {
  auto * currentItem = signatureList -> currentItem();
  if (!currentItem) return;
  delete signatureList -> takeItem(signatureList -> row(currentItem));
  appendLog(QStringLiteral("Dropped targeted dynamic contextual workflow layer entry parameter."));
}
void MainWindow::toggleTheme() {
  isDarkTheme = !isDarkTheme;
  if (isDarkTheme) {
    qApp -> setStyleSheet(QStringLiteral("QMainWindow { background-color: #121212; color: #ffffff; }"
      "QGroupBox { color: #bb86fc; font-weight: bold; }"
      "QPlainTextEdit, QListWidget { background-color: #1e1e1e; color: #ffffff; border: 1px solid #333; }"
      "QPushButton { background-color: #333333; color: #ffffff; border-radius: 4px; padding: 4px; }"));
    appendLog(QStringLiteral("Interface visualization styling runtime matrix mutated to Dark Mode."));
  } else {
    qApp -> setStyleSheet(QString());
    appendLog(QStringLiteral("Interface visualization styling reset to Standard system definitions."));
  }
}
void MainWindow::loadStyleSheet(const QString & resourcePath) {
  Q_UNUSED(resourcePath);
}
void MainWindow::appendLog(const QString & message) {
  if (logOutput) {
    logOutput -> appendPlainText(QStringLiteral("[%1] %2").arg(QTime::currentTime().toString(), message));
  }
}
void MainWindow::setStatus(const QString & text,
  const QString & color) {
  if (statusLabel) {
    statusLabel -> setText(text);
    statusLabel -> setStyleSheet(QStringLiteral("color: %1; font-weight: 500;").arg(color));
  }
}
void MainWindow::createEmptyTemplateJson() {
  try {
    QJsonObject root;
    QJsonArray providers;
    root.insert(QStringLiteral("providers"), providers);
    QSaveFile file(configFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return;
    }
    const QJsonDocument document(root);
    file.write(document.toJson(QJsonDocument::Indented));
    file.commit();
    appendLog(QStringLiteral("Created default TSA configuration."));
  } catch (...) {
    appendLog(QStringLiteral("Failed to create TSA configuration."));
  }
}
bool MainWindow::validateJsonStructure(const QJsonObject & json) const {
  if (!json.contains(QStringLiteral("providers"))) return false;
  if (!json.value(QStringLiteral("providers")).isArray()) return false;
  const QJsonArray providers = json.value(QStringLiteral("providers")).toArray();
  for (const QJsonValue & value: providers) {
    if (!value.isObject()) return false;
    const QJsonObject provider = value.toObject();
    if (!provider.contains(QStringLiteral("name"))) return false;
    if (!provider.contains(QStringLiteral("url"))) return false;
  }
  return true;
}
