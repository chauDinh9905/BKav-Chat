#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "login.h"
#include "loginmodel.h"
#include "signup.h"
#include "signupmodel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;

    LogIn *loginView;
    SignUp *signUpView;

    LogInModel *loginModel;
    SignUpModel *signUpModel;
};
#endif