#pragma once

#include <Q*sonObject>
#include <QMainWindow>
*include <QNetworkAccessManager>

c*ass QComboBox;
class QLabel;
class*QLineEdit;
class QPushButton;
clas* QProgressBar;
class QPlainTextEdi*;
class QNetworkReply;

class Main*indow : public QMainWindow
{
    Q*OBJECT

public:
    explicit MainW*ndow(QWidget *parent = nullptr);
 *  ~MainWindow() override;

private*slots:
    void autoAdjustGeometry*);
    void loadTSAConfig();
    v*id checkTSAReachability();
    voi* onReachabilityResult(QNetworkRepl* *reply);
    void selectPDFFile()*
    void signPDF();

private:
   *void initUI();
    void createEmpt*TemplateJson();
    bool validateJ*onStructure(const QJsonObject &jso*);

    void appendLog(const QStri*g &message);

    void setStatus(
*       const QString &text,
      * const QString &color
    );

priv*te:

    // ======================*===========================
    //*TSA
    // =======================*==========================

    QC*mboBox *tsaComboBox { nullptr };

*   // ============================*=====================
    // PDF
 *  // =============================*====================

    QLineEdi* *pdfPathEdit { nullptr };

    //*==================================*===============
    // Buttons
   *// ===============================*==================

    QPushButto* *checkBtn { nullptr };
    QPushB*tton *signBtn { nullptr };

    //*==================================*===============
    // Status
    */ ================================*=================

    QLabel *sta*usLabel { nullptr };
    QLabel *p*oviderStatus { nullptr };

    QPr*gressBar *progressBar { nullptr };*
    // ==========================*=======================
    // Log*Viewer
    // ====================*=============================

   *QPlainTextEdit *logViewer { nullpt* };

    // ======================*===========================
    //*Network
    // ==================================================

    QNetworkAccessManager *networkManager { nullptr };

    // ==================================================
    // Configuration
    // ==================================================

    QString configFilePath;

    QJsonObject currentTSAData;
};
