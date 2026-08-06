#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QTextStream>

#include "MainWindow.h"

namespace
{
    void writeCrashLog(const QString& message)
    {
        QFile file("crash.log");

        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream stream(&file);

            stream << message << Qt::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName(
        "PDF Integrity Friend");

    app.setApplicationVersion(
        "1.0-Beta");

    app.setOrganizationName(
        "Ratio Juris");

    app.setOrganizationDomain(
        "github.com");

    app.setWindowIcon(
        QIcon(":/generated/app-icon-512.png"));

    try
    {
        MainWindow window;

        window.show();

        return app.exec();
    }
    catch (const std::exception& ex)
    {
        writeCrashLog(
            QString(
                "Fatal Exception: %1")
                .arg(ex.what()));

        QMessageBox::critical(
            nullptr,
            "PDF Integrity Friend",
            QString(
                "A fatal error occurred.\n\n%1")
                .arg(ex.what()));

        return EXIT_FAILURE;
    }
    catch (...)
    {
        writeCrashLog(
            "Unknown fatal exception.");

        QMessageBox::critical(
            nullptr,
            "PDF Integrity Friend",
            "An unknown fatal error occurred.");

        return EXIT_FAILURE;
    }
}
