#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setStyleSheet("QMainWindow, QWidget { background-color: white; }");
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
   setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    loginModel = new LogInModel(this);
    signUpModel = new SignUpModel(this);

    loginView = new LogIn(loginModel, this);
    signUpView = new SignUp(signUpModel, this);

    // currentIndex của loginView sẽ là 0, signUpView sẽ là 1
    stackedWidget->addWidget(loginView);
    stackedWidget->addWidget(signUpView);

    connect(loginView, &LogIn::signUpRequest, this, [=]() {
        stackedWidget->setCurrentWidget(signUpView); // Chuyển sang màn SignUp mượt mà
    });

    // Giả sử bên SignUp bạn có signal backToLoginRequested
    connect(signUpView, &SignUp::backToLogInRequest, this, [=]() {
        stackedWidget->setCurrentWidget(loginView);
    });

    // Mặc định ban đầu mở App là hiện trang Login (vị trí 0)
    stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow() {}