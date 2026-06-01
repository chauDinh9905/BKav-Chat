#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QSqlDatabase>

class DatabaseManager{
public:
    //pattern singleton
    static DatabaseManager& instance();
    // check if whether connect succeed or fail
    bool connectToDatabase();
    QSqlDatabase& getDatabase();
private:
    //constrcutor
    DatabaseManager();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
