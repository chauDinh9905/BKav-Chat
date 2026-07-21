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
#include "nicknamecontroller.h"

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
    LogInModel *loginModel;
    SignUpModel *signUpModel;
    DashboardModel *dashboardModel;
    ChatModel *chatModel;
public:
    void connectDashboardSignals();
protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif