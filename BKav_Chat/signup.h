#ifndef SIGNUP_H
#define SIGNUP_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>

namespace Ui {
class SignUp;
}

class SignUp : public QWidget
{
    Q_OBJECT

public:
    explicit SignUp(QWidget *parent = nullptr);
    ~SignUp();

private:
    Ui::SignUp *ui;

    QLabel *title;
    QLabel *name;
    QLabel *account;
    QLabel *password;
    QLabel *password1;

    QLineEdit *textName;
    QLineEdit *textAccount;
    QLineEdit *textPassword;
    QLineEdit *textPassword1;

    QPushButton *taoTaiKhoan;

    QVBoxLayout *mainLayout;
    QGridLayout *gridLayout;
};

#endif // SIGNUP_H
