#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include "loginmodel.h"

namespace Ui {
class LogIn;
}

class LogIn : public QWidget
{
    Q_OBJECT

public:
    explicit LogIn(LogInModel *model,QWidget *parent = nullptr);
    ~LogIn();

private:
    Ui::LogIn *ui;

    LogInModel *model;

    QLabel *title;
    QLabel *account;
    QLabel *password;
    QLabel *error;

    QLineEdit *textAccount;
    QLineEdit *textPassword;

    QCheckBox *rememberPass;

    QPushButton *logIn;
    QPushButton *signUp;

    QVBoxLayout *mainLayout;
    QGridLayout *gridLayout;

signals:
    void logInSuccess();
    void signUpRequest();
    void rememberAccount();
public slots:
    void rememberInfoClicked();
    void logInClicked();
    void signUpClicked();
};

#endif // LOGIN_H
