#include "errorlogin.h"
#include "ui_errorlogin.h"

ErrorLogIn::ErrorLogIn(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("BKav Chat");
    this->setFixedSize(650, 400);
    this->setStyleSheet("background-color: white;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

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
    password->setStyleSheet("font-size: 14px; color: black;");
    textPassword = new QLineEdit(this);
    textPassword->setEchoMode(QLineEdit::Password); // Ẩn mật khẩu bằng dấu chấm tròn
    textPassword->setStyleSheet("padding: 6px; border: 1px solid ; border-radius: 4px; font-size: 14px;");

    gridLayout->addWidget(account, 0, 0);
    gridLayout->addWidget(textAccount, 0, 1);
    gridLayout->addWidget(password, 1, 0);
    gridLayout->addWidget(textPassword, 1, 1);
    mainLayout->addLayout(gridLayout);

    remeberPass = new QCheckBox("Nhớ tài khoản và mật khẩu", this);
    remeberPass->setStyleSheet("font-size: 13px; color: #555555;");
    mainLayout->addWidget(remeberPass);

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
        "QPushButton:hover {"
        "   background-color: #1565c0;" // Đậm màu khi di chuột vào
        "}"
        "QPushButton:pressed {"
        "   background-color: #0d47a1;" // Đậm hơn nữa khi bấm giữ nút
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

    thongBao = new QLabel("Bạn nhập sai tên tài khoản hoặc mật khẩu", this);
    thongBao->setStyleSheet("font-size: 14px; color: red;");
    thongBao->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(thongBao);

}

ErrorLogIn::~ErrorLogIn()
{

}
