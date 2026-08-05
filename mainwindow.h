#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void autoAdjustGeometry();
    void loadTSAConfig();
    void checkTSAReachability();
    void onReachabilityResult(QNetworkReply* reply);
    void selectPDFFile();
    void signPDF();

private:
    void initUI();
    void createDefaultJson();
    bool validateJsonStructure(const QJsonObject& json);

    // UI Elements
    QComboBox* tsaComboBox;
    QLineEdit* pdfPathEdit;
    QPushButton* checkBtn;
    QPushButton* signBtn;
    QLabel* statusLabel;

    // Network & Config
    QNetworkAccessManager* networkManager;
    QString configFilePath;
    QJsonObject currentTSAData;
};
