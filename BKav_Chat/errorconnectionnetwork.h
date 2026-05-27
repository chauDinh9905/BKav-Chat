#ifndef ERRORCONNECTIONNETWORK_H
#define ERRORCONNECTIONNETWORK_H

#include <QDialog> //thư viện để tạo hộp thoại dạng popup
#include <QLabel> // thư viện dùng chữ hiển thị
#include <QPushButton> // thư viện dùng tạo nút bấm
#include <QVBoxLayout> // thư viện để tạo layout


namespace Ui {
class ErrorConnectionNetwork;
}

class ErrorConnectionNetwork : public QDialog
{
    Q_OBJECT //bắt buộc trong Qt

public:
    explicit ErrorConnectionNetwork(QWidget *parent = nullptr);
    ~ErrorConnectionNetwork();

private:
    QLabel *labelMessage;
    QPushButton *buttonOk;
    QVBoxLayout *mainLayout;
};

#endif // ERRORCONNECTIONNETWORK_H
