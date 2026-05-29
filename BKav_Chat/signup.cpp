#include "signup.h"
#include "ui_signup.h"
#include <QMessageBox>

SignUp::SignUp( SignUpModel *model ,QWidget *parent)
    : QWidget(parent), model(model)
{
    this->setWindowTitle("BKav Chat");
    this->setFixedSize(650, 400);
    this->setStyleSheet("background-color: white;");


    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    backToLogIn = new QPushButton("←", this);
    backToLogIn->setStyleSheet("QPushButton { text-align: left; background: none; border: none; color: black; font-size: 18px}");
    backToLogIn->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(backToLogIn);

    title = new QLabel("Tạo tài khoản");
    title->setStyleSheet("color: black; font-size: 24px; font-weight: bold;");
    title->setText("<span style='background-color: LightGrey; padding: 2px 5px;'>Tạo tài khoản</span>");
    title->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(title);

    gridLayout = new QGridLayout();
    gridLayout->setSpacing(15);

    name = new QLabel("Tên hiển thị", this);
    name->setStyleSheet("font-size: 14px; color: black;");
    textName = new QLineEdit(this);
    textName->setStyleSheet("padding: 6px; border: 1px solid; border-radius: 4px; font-size: 14px;");

    account = new QLabel("Tài khoản", this);
    account->setStyleSheet("font-size: 14; color: black;");
    textAccount = new QLineEdit(this);
    textAccount->setStyleSheet("padding: 6px; border: 1px solid; border-radius: 4px; font-size: 14px;");

    password = new QLabel("Mật khẩu", this);
    password->setStyleSheet("font-size: 14px; color: black;");
    textPassword = new QLineEdit(this);
    textPassword->setStyleSheet("padding: 6px; border: 1px solid; border-radius: 4px; font-size: 14px");
    textPassword->setEchoMode(QLineEdit::Password);

    password1 = new QLabel("Nhập lại mật khẩu", this);
    password1->setStyleSheet("font-size: 14px; color: black;");
    textPassword1 = new QLineEdit(this);
    textPassword1->setStyleSheet("padding: 6px; border: 1px solid; border-radius: 4px; font-size: 14px");
    textPassword1->setEchoMode(QLineEdit::Password);

    gridLayout->addWidget(name, 0, 0);
    gridLayout->addWidget(textName, 0, 1);
    gridLayout->addWidget(account, 1, 0);
    gridLayout->addWidget(textAccount, 1, 1);
    gridLayout->addWidget(password, 2, 0);
    gridLayout->addWidget(textPassword, 2, 1);
    gridLayout->addWidget(password1, 3, 0);
    gridLayout->addWidget(textPassword1, 3, 1);
    mainLayout->addLayout(gridLayout);

    error = new QLabel("", this);
    error->setStyleSheet("font-size: 14px; color: red;");
    error->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(error);

    taoTaiKhoan = new QPushButton("Tạo tài khoản", this);
    taoTaiKhoan->setCursor(Qt::PointingHandCursor);
    taoTaiKhoan->setStyleSheet("background-color: blue; color: white; font-size: 14px");
    mainLayout->addWidget(taoTaiKhoan);

    connect(taoTaiKhoan, &QPushButton::clicked, this, &SignUp::taoTaiKhoanClicked);
    connect(backToLogIn, &QPushButton::clicked, this, &SignUp::backToLogInRequest);
}

void SignUp::taoTaiKhoanClicked(){
    if(!model) return;
    model->displayName = textName->text();
    model->userName = textAccount->text();
    model->password = textPassword->text();
    model->confirmPassword = textPassword1->text();

    if(model->checkCredentials()){
        model->registerOnServer();
        //QMessageBox::information(this, "Thông báo", "Đăng ký tài khoản thành công!");
        error->setText("");
        emit signUpSuccess();
    }else{
        //QMessageBox::warning(this, "Lỗi", "Thông tin không hợp lệ hoặc mật khẩu không khớp!");
        error->setText("Tài khoản đã tồn tại hoặc thông tin không hợp lệ");
    }
}

SignUp::~SignUp()
{

}
