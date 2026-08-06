#pragma once

#include <QJsonObject>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QString>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QPlainTextEdit;
class QNetworkReply;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void autoAdjustGeometry();
    void loadTSAConfig();
    void checkTSAReachability();
    void onReachabilityResult(QNetworkReply *reply);
    void selectPDFFile();
    void signPDF();

private:
    // UI
    void initUI();

    // Configuration
    void createEmptyTemplateJson();
    bool validateJsonStructure(const QJsonObject &json);

    // Helpers
    void appendLog(const QString &message);
    void setStatus(
        const QString &text,
        const QString &color = "#1565c0"
    );

private:

    //--------------------------------------------------
    // TSA
    //--------------------------------------------------

    QComboBox *tsaComboBox{nullptr};

    //--------------------------------------------------
    // PDF
    //--------------------------------------------------

    QLineEdit *pdfPathEdit{nullptr};

    //--------------------------------------------------
    // Buttons
    //--------------------------------------------------

    QPushButton *checkBtn{nullptr};
    QPushButton *signBtn{nullptr};

    //--------------------------------------------------
    // Status
    //--------------------------------------------------

    QLabel *statusLabel{nullptr};
    QLabel *providerStatus{nullptr};

    //--------------------------------------------------
    // Progress
    //--------------------------------------------------

    QProgressBar *progressBar{nullptr};

    //--------------------------------------------------
    // Log Viewer
    //--------------------------------------------------

    QPlainTextEdit *logViewer{nullptr};

    //--------------------------------------------------
    // Network
    //--------------------------------------------------

    QNetworkAccessManager *networkManager{nullptr};

    //--------------------------------------------------
    // Configuration
    //--------------------------------------------------

    QString configFilePath;

    QJsonObject currentTSAData;
};
