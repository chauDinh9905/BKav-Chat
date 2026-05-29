#include "login.h"

LogIn::LogIn(LogInModel *model, QWidget *parent)
    : QWidget(parent), model(model)
{
    this->setWindowTitle("BKav Chat");
    this->setFixedSize(650, 400);
    this->setStyleSheet("background-color: white;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(10);

    title = new QLabel("Bkav Chat", this);
    title->setStyleSheet("color: blue; font-size: 24px; font-weight: bold;");
    title->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(title);

    gridLayout = new QGridLayout();
    gridLayout->setSpacing(15);

    account = new QLabel("Tài khoản", this);
    account->setStyleSheet("font-size: 14px; color: black;");
    textAccount = new QLineEdit(this);
    textAccount->setStyleSheet("padding: 6px; border: 1px solid ; border-radius: 4px; font-size: 14px;");

    password = new QLabel("Mật khẩu", this);
    password->setStyleSheet("font-size: 14px; color: black+;");
    textPassword = new QLineEdit(this);
    textPassword->setEchoMode(QLineEdit::Password); // Ẩn mật khẩu bằng dấu chấm tròn
    textPassword->setStyleSheet("padding: 6px; border: 1px solid ; border-radius: 4px; font-size: 14px;");
    textPassword->setEchoMode(QLineEdit::Password);

    gridLayout->addWidget(account, 0, 0);
    gridLayout->addWidget(textAccount, 0, 1);
    gridLayout->addWidget(password, 1, 0);
    gridLayout->addWidget(textPassword, 1, 1);
    mainLayout->addLayout(gridLayout);

    rememberPass = new QCheckBox("Nhớ tài khoản và mật khẩu", this);
    rememberPass->setStyleSheet("font-size: 13px; color: #555555;");
    mainLayout->addWidget(rememberPass);

    logIn = new QPushButton("Đăng nhập", this);
    logIn->setCursor(Qt::PointingHandCursor); // Đổi con trỏ chuột thành hình bàn tay khi di qua
    logIn->setStyleSheet(
        "QPushButton {"
        "   background-color: blue;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 15px;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 10px;"
        "}"
        );
    mainLayout->addWidget(logIn);

    signUp = new QPushButton("Đăng ký", this);
    signUp->setCursor(Qt::PointingHandCursor);
    signUp->setStyleSheet(
        "QPushButton {"
        "   color: blue;"
        "   background: none;"
        "   border: none;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   text-decoration: underline;" // Gạch chân khi di chuột qua
        "}"
        );
    mainLayout->addWidget(signUp);

    error = new QLabel("", this);
    error->setStyleSheet("font-size: 14px; color: red");
    error->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(error);

    connect(rememberPass, &QCheckBox::clicked, this, &LogIn::rememberInfoClicked);
    connect(logIn, &QPushButton::clicked, this, &LogIn::logInClicked);
    connect(signUp, &QPushButton::clicked, this, &LogIn::signUpClicked);
    connect(model, &LogInModel::authenticationSucceeded, this, [=]() {
        // Server OK -> View phát tín hiệu báo cho main.cpp đổi sang màn hình chính A
        emit logInSuccess();
    });

    connect(model, &LogInModel::authenticationFailed, this, [=]() {
        // Server báo lỗi -> Hiện lên QLabel error của giao diện đăng nhập
        error->setText("Sai tên tài khoản hoặc mật khẩu");
    });
}

void LogIn::rememberInfoClicked(){
     model->rememberInfo = true;
    emit rememberAccount();
}

void LogIn::logInClicked(){
    model->account = textAccount->text();
    model->password = textPassword->text();
    model->rememberInfo = rememberPass->isChecked();

    if(model->validateInfo()){
        error->setText("");
        model->authenticateWithServer();
    }else{
        error->setText("Thông tin tài khoản không hợp lệ");
    }
}

void LogIn::signUpClicked(){
    emit signUpRequest();
}


LogIn::~LogIn()
{

}
