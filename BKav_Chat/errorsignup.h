#ifndef ERRORSIGNUP_H
#define ERRORSIGNUP_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>

namespace Ui {
class ErrorSignUp;
}

class ErrorSignUp : public QWidget
{
    Q_OBJECT

public:
    explicit ErrorSignUp(QWidget *parent = nullptr);
    ~ErrorSignUp();

private:
    QLabel *title;
    QLabel *name;
    QLabel *account;
    QLabel *password;
    QLabel *password1;
    QLabel *thongBao;

    QLineEdit *textName;
    QLineEdit *textAccount;
    QLineEdit *textPassword;
    QLineEdit *textPassword1;

    QPushButton *taoTaiKhoan;

    QVBoxLayout *mainLayout;
    QGridLayout *gridLayout;
    Ui::ErrorSignUp *ui;
};

#endif // ERRORSIGNUP_H
