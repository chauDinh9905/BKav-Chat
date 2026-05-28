#ifndef SIGNUP_H
#define SIGNUP_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include "signupmodel.h"

namespace Ui {
class SignUp;
}

class SignUp : public QWidget
{
    Q_OBJECT

public:
    explicit SignUp(SignUpModel *model ,QWidget *parent = nullptr);
    ~SignUp();

private:

    SignUpModel *model;

    QLabel *title;
    QLabel *name;
    QLabel *account;
    QLabel *password;
    QLabel *password1;
    QLabel *error;

    QLineEdit *textName;
    QLineEdit *textAccount;
    QLineEdit *textPassword;
    QLineEdit *textPassword1;

    QPushButton *taoTaiKhoan;

    QVBoxLayout *mainLayout;
    QGridLayout *gridLayout;

signals:
    void signUpSuccess();

private slots:
    void taoTaiKhoanClicked();
};

#endif // SIGNUP_H
