#include "mainwindow.h"
#include "SocketManager.h"
#include "appconfig.h"
#include "DatabaseManager.h"
#include "SecurityUtils.h"
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setStyleSheet("QMainWindow, QWidget { background-color: white; }");
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    loginModel = new LogInModel(this);
    signUpModel = new SignUpModel(this);
    //dashboardModel = new DashboardModel(this);


    loginView = new LogIn(loginModel, this);
    signUpView = new SignUp(signUpModel, this);
    //dashboardView = new Dashboard(dashboardModel, this);

    // currentIndex của loginView sẽ là 0, signUpView sẽ là 1
    stackedWidget->addWidget(loginView);
    stackedWidget->addWidget(signUpView);
    //stackedWidget->addWidget(dashboardView);
    connect(loginView, &LogIn::signUpRequest, this, [=]() {
        stackedWidget->setCurrentWidget(signUpView);
    });

    connect(loginView,
            &LogIn::logInSuccess,
            this,
            [this](qint64 loggedInUserId)
            {
                dashboardModel = new DashboardModel(this, QString::number(loggedInUserId));
                dashboardView = new Dashboard(dashboardModel, this);
                DatabaseManager::instance().init(loggedInUserId);

                // Lấy token đã lưu để derive key
                QString configPath = AppConfig::instance().getConfigFilePath();
                QSettings settings(configPath, QSettings::IniFormat);
                QString token = settings.value("auth/token").toString();

                QString localKey = SecurityUtils::generateLocalKey(token, loggedInUserId);
                DatabaseManager::instance().setEncryptionKey(localKey);

                stackedWidget->addWidget(dashboardView);
                stackedWidget->setCurrentWidget(dashboardView);

                connectDashboardSignals();
                SocketManager::instance().connectToServer();
            });

    connect(signUpView, &SignUp::backToLogInRequest, this, [=](const QString &username) {
        loginView->setAccountName(username);
        stackedWidget->setCurrentWidget(loginView);
            });
                    // Mặc định ban đầu mở App là hiện trang Login (vị trí 0)
    stackedWidget->setCurrentIndex(0);

    // Mặc định ban đầu mở App là hiện trang Login (vị trí 0)

}

void MainWindow::connectDashboardSignals()
{
    connect(dashboardView,
            &Dashboard::logOutRequest,
            this,
            [this]()
            {
                AppConfig::instance().setProfile("default");
                stackedWidget->setCurrentWidget(loginView);

                stackedWidget->removeWidget(dashboardView);
                delete dashboardView;
                dashboardView = nullptr;
            });

    connect(dashboardView,
            &Dashboard::openChatRequest,
            this,
            [this](const FriendInfo &friendInfo)
            {
                if(chatView)
                {
                    stackedWidget->removeWidget(chatView);
                    chatView->deleteLater();
                    chatView = nullptr;
                }

                chatView = new Chat(
                    dashboardView->getMyId(),
                    friendInfo.friendId,
                    friendInfo.displayName,
                    friendInfo.avatarUrl
                    );



                stackedWidget->addWidget(chatView);
                stackedWidget->setCurrentWidget(chatView);

                connect(chatView,
                        &Chat::closeRequested,
                        this,
                        [this]()
                        {
                            stackedWidget->setCurrentWidget(dashboardView);

                            stackedWidget->removeWidget(chatView);
                            chatView->deleteLater();
                            chatView = nullptr;
                        });
            });
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (dashboardView) {
        SocketManager::instance().unregisterUser(dashboardView->getMyId());
    }
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow() {}