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
    void initUI();
    void createEmptyTemplateJson();
    bool validateJsonStructure(const QJsonObject &json);

    void appendLog(const QString &message);
    void setStatus(const QString &text, const QString &color = "#1565c0");

private:
    QComboBox *tsaComboBox{nullptr};
    QLineEdit *pdfPathEdit{nullptr};

    QPushButton *checkBtn{nullptr};
    QPushButton *signBtn{nullptr};

    QLabel *statusLabel{nullptr};
    QLabel *providerStatus{nullptr};

    QProgressBar *progressBar{nullptr};
    QPlainTextEdit *logViewer{nullptr};

    QNetworkAccessManager *networkManager{nullptr};

    QString configFilePath;
    QJsonObject currentTSAData;
};
