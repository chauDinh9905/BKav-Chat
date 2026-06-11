#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QCoreApplication>
#include <QSettings>

class AppConfig {
public:
    static AppConfig& instance() {
        static AppConfig instance;
        return instance;
    }

    // Hàm load file ini khi app vừa khởi động
    void loadConfig() {
        // Lấy đường dẫn thư mục chứa file chạy của App
        QString appDir = QCoreApplication::applicationDirPath();
        QString filePath = appDir + "/config.ini";

        QSettings settings(filePath, QSettings::IniFormat);

        // Đọc giá trị, nếu file không tồn tại sẽ lấy giá trị mặc định (localhost:3000)
        serverIp = settings.value("Server/Ip", "localhost").toString();
        serverPort = settings.value("Server/Port", "3000").toString();
    }

    // Hàm tiện ích để lấy ra Base URL hoàn chỉnh cho API
    QString getBaseUrl() const {
        return QString("http://%1:%2/api").arg(serverIp, serverPort);
    }

private:
    AppConfig() = default;
    QString serverIp;
    QString serverPort;
};

#endif // APPCONFIG_H