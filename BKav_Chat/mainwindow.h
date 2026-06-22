#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "login.h"
#include "loginmodel.h"
#include "signup.h"
#include "signupmodel.h"
#include "dashboard.h"
#include "dashboardmodel.h"
#include "chat.h"

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
    Dashboard *dashboardView;
    Chat *chatView = nullptr;

    LogInModel *loginModel;
    SignUpModel *signUpModel;
    DashboardModel *dashboardModel;
    ChatModel *chatModel;
private:
    void connectDashboardSignals();
};
#endif