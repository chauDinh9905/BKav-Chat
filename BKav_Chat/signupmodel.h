#ifndef SIGNUPMODEL_H
#define SIGNUPMODEL_H

#include <QObject>

class SignUpModel : public QObject
{
    Q_OBJECT

public:
    explicit SignUpModel(QObject *parent = nullptr);

    QString displayName;
    QString userName;
    QString password;
    QString confirmPassword;

    bool checkCredentials(); // Kiểm tra và xác thực thông tin
    bool registerOnServer(); // Ở hàm trên nếu trả về true thì gửi lên server kiểm tra xem có người dùng nào xung đột thông tin hay không
};

#endif // SIGNUPMODEL_H
