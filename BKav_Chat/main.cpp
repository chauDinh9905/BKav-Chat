#include "signup.h"
#include "login.h"
#include "errorlogin.h"
#include "errorconnectionnetwork.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "loginmodel.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "BKav_Chat_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();

    return QApplication::exec();
}
