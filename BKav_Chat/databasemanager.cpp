#include "databasemanager.h"
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

bool DatabaseManager::connectToDatabase()
{
    db.setDatabaseName(
        "Driver={MySQL ODBC 9.7 Unicode Driver};"
        "Server=localhost;"
        "Database=BKav_chat;"
        "User=root;"
        "Password=Chau@123;"
        "Port=3306;"
        );

    if (db.open()) {
        qDebug() << "Kết nối MySQL thành công!";
        return true;
    } else {
        qDebug() << "Kết nối thất bại:" << db.lastError().text();
        return false;
    }
}

QSqlDatabase& DatabaseManager::getDatabase()
{
    return db;
}