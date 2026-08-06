#include <QApplication>
#include <QMessageBox>
#include <QIcon>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("PDF Integrity Friend");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("RatioJuris");
    app.setOrganizationDomain("github.com");

    try
    {
        MainWindow window;
        window.show();

        return app.exec();
    }
    catch (const std::exception& ex)
    {
        QMessageBox::critical(
            nullptr,
            "Application Error",
            QString("A fatal error occurred:\n%1").arg(ex.what())
        );

        return EXIT_FAILURE;
    }
    catch (...)
    {
        QMessageBox::critical(
            nullptr,
            "Application Error",
            "An unknown fatal error occurred."
        );

        return EXIT_FAILURE;
    }
}
