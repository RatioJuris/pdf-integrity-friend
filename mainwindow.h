#pragma once

#include <QJsonObject>
#include <QMainWindow>
#include <QNetworkAccessManager>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QNetworkReply;
class QProgressBar;

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

private:
    // UI Controls
    QComboBox *tsaComboBox { nullptr };
    QLineEdit *pdfPathEdit { nullptr };
    QPushButton *checkBtn { nullptr };
    QPushButton *signBtn { nullptr };
    QLabel *statusLabel { nullptr };
    QProgressBar *progressBar { nullptr };

    // Network
    QNetworkAccessManager *networkManager { nullptr };

    // Configuration
    QString configFilePath;
    QJsonObject currentTSAData;
};
