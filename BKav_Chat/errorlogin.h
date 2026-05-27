#ifndef ERRORLOGIN_H
#define ERRORLOGIN_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>

namespace Ui {
class LogIn;
}

class ErrorLogIn : public QWidget
{
    Q_OBJECT

public:
    explicit ErrorLogIn(QWidget *parent = nullptr);
    ~ErrorLogIn();

private:
    Ui::LogIn *ui;
    QLabel *title;
    QLabel *account;
    QLabel *password;
    QLabel *thongBao;

    QLineEdit *textAccount;
    QLineEdit *textPassword;

    QCheckBox *remeberPass;

    QPushButton *logIn;
    QPushButton *signUp;

    QVBoxLayout *mainLayout;
    QGridLayout *gridLayout;
};

#endif // ERRORLOGIN_H
