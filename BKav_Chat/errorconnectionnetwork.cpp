#include "errorconnectionnetwork.h"
#include "ui_errorconnectionnetwork.h"

ErrorConnectionNetwork::ErrorConnectionNetwork(QWidget *parent)
    : QDialog(parent)
{
    this->setFixedSize(383, 177);
    this->setStyleSheet("background-color: white;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 40, 30, 30); // Lề xung quanh
    mainLayout->setSpacing(25);

    labelMessage = new QLabel("Kết nối mạng không ổn định, bạn vui\nlòng thử lại sau", this);
    labelMessage->setAlignment(Qt::AlignCenter);
    labelMessage->setStyleSheet(
        "color: black;"
        "font-size: 15px;"
        "font-family: 'Segoe UI', Arial, sans-serif;"
        "line-height: 1.4;" //khoảng cách giữa 2 dòng chữ
        );
    mainLayout->addWidget(labelMessage);

    buttonOk = new QPushButton("Ok", this);
    buttonOk->setCursor(Qt::PointingHandCursor); // hiệu ứng thay đổi con trỏ chuột
    buttonOk->setFixedWidth(120);
    buttonOk->setStyleSheet(
        "QPushButton {"
        "   background-color: blue;"
        "   color: white;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "}"
        );
    mainLayout->addWidget(buttonOk, 0, Qt::AlignCenter);

    connect(buttonOk, &QPushButton::clicked, this, &QDialog::accept); // (tk gửi, tín hiệu, tk nhận, hành động xử lý)
}

ErrorConnectionNetwork::~ErrorConnectionNetwork()
{

}
