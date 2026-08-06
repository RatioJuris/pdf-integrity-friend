#pragma once

#include <QJsonObject>
#include <QMainWindow>
#include <QString>
#include <QVector>

class QAction;
class QLabel;
class QSplitter;
class QListWidget;
class QPushButton;
class QProgressBar;
class QPlainTextEdit;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QPdfDocument;
class QPdfView;
class QNetworkAccessManager;
class QNetworkReply;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void openPdf();
    void savePdf();
    void loadTSAConfig();

    void checkTSAReachability();
    void onReachabilityResult(QNetworkReply *reply);

    void addVisibleSignature();
    void removeSelectedSignature();

    void toggleTheme();

private:
    void initializeUi();
    void initializeMenuBar();
    void initializeStatusBar();

    void loadStyleSheet(const QString &resourcePath);
    void appendLog(const QString &message);
    void setStatus(const QString &text, const QString &color = QStringLiteral("#1565c0"));

    void createEmptyTemplateJson();
    
    [[nodiscard]]
    bool validateJsonStructure(const QJsonObject &json) const;

private:
    QSplitter *mainSplitter{nullptr};

    QPdfDocument *pdfDocument{nullptr};
    QPdfView *pdfView{nullptr};
    QListWidget *pageList{nullptr};

    QListWidget *signatureList{nullptr};
    QPushButton *addSignatureButton{nullptr};
    QPushButton *removeSignatureButton{nullptr};

    QComboBox *tsaComboBox{nullptr};
    QPushButton *checkTsaButton{nullptr};

    QCheckBox *disableEditingCheckBox{nullptr};
    QCheckBox *disableCopyCheckBox{nullptr};
    QCheckBox *disablePrintCheckBox{nullptr};
    QCheckBox *disableCommentCheckBox{nullptr};

    QLabel *statusLabel{nullptr};
    QLabel *tsaStatusLabel{nullptr};
    QProgressBar *progressBar{nullptr};
    QPlainTextEdit *logOutput{nullptr};

    QNetworkAccessManager *networkManager{nullptr};

    QString currentPdfFile;
    QString configFilePath;
    QJsonObject currentTSAData;
    bool isDarkTheme{false};
};
