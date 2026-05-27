#ifndef LOGIN_H
#define LOGIN_H

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

class LogIn : public QWidget
{
    Q_OBJECT

public:
    explicit LogIn(QWidget *parent = nullptr);
    ~LogIn();

private:
    Ui::LogIn *ui;
    QLabel *title;
    QLabel *account;
    QLabel *password;

    QLineEdit *textAccount;
    QLineEdit *textPassword;

    QCheckBox *remeberPass;

    QPushButton *logIn;
    QPushButton *signUp;

    QVBoxLayout *mainLayout;
    QGridLayout *gridLayout;
};

#endif // LOGIN_H
