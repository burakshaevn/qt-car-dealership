#include "AppServices.h"
#include "MainWindow.h"
#include "ThemeManager.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    // The interface, prices and dates are Russian regardless of the OS locale.
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));
    QApplication::setOrganizationName(QStringLiteral("burakshaevn"));
    QApplication::setApplicationName(QStringLiteral("qt-car-dealership"));
    // No applicationDisplayName on purpose: Qt would append it to every window title.
    QApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::translate("main", "Car dealership client"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption kDatabaseOption(
        {QStringLiteral("d"), QStringLiteral("database")},
        QApplication::translate("main", "Path to the SQLite database (created and migrated if missing)."),
        QStringLiteral("path"));
    parser.addOption(kDatabaseOption);
    parser.process(app);

    ThemeManager::instance().initialize(app);

    AppServices services;
    if (!services.initialize(parser.value(kDatabaseOption))) {
        QMessageBox::critical(nullptr, QApplication::translate("main", "База данных"), services.errorString());
        return EXIT_FAILURE;
    }

    MainWindow window(services);
    window.show();
    return app.exec();
}
