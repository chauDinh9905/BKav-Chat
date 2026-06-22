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

    QString getServerIp() const {
        return serverIp;
    }

    QString getServerPort() const {
        return serverPort;
    }

    QString getConfigFilePath() const {
        return QCoreApplication::applicationDirPath() + "/" + configFileName;
    }
    void setProfile(const QString &profileName) {
        if (!profileName.isEmpty()) {
            configFileName = QString("config_%1.ini").arg(profileName);
        } else {
            configFileName = "config.ini"; // Mặc định nếu không truyền gì
        }
        // Sau khi đổi tên file, load lại cấu hình từ file đó
        loadConfig();
    }
private:
    AppConfig() = default;
    QString serverIp;
    QString serverPort;
    QString configFileName;
};

#endif // APPCONFIG_H