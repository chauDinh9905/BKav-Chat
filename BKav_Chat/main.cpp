#include "appconfig.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AppConfig::instance().loadConfig();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "BKav_Chat_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    /*
    if (!DatabaseManager::instance().connectToDatabase())
    {
        qDebug() << "Cannot connect database";
        return -1;
    }*/
    MainWindow w;
    w.show();

    return QApplication::exec();
}

