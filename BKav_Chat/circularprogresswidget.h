#ifndef CIRCULARPROGRESSWIDGET_H
#define CIRCULARPROGRESSWIDGET_H

#include <QWidget>

class CircularProgressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CircularProgressWidget(QWidget *parent = nullptr);

    void setProgress(int percent); // 0..100, -1 = chưa xác định (indeterminate)
    void setStatusText(const QString &text); // vd "Đang tải...", "Hoàn tất"

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_percent = 0;
    QString m_statusText;
};

#endif // CIRCULARPROGRESSWIDGET_H