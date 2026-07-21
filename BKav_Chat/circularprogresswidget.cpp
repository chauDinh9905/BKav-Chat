#include "circularprogresswidget.h"
#include <QPainter>
#include <QPen>

CircularProgressWidget::CircularProgressWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(110, 110);
    setAttribute(Qt::WA_TranslucentBackground);
}

void CircularProgressWidget::setProgress(int percent)
{
    m_percent = qBound(0, percent, 100);
    update();
}

void CircularProgressWidget::setStatusText(const QString &text)
{
    m_statusText = text;
    update();
}

void CircularProgressWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int pad = 8;
    QRectF circleRect(pad, pad, width() - 2 * pad, height() - 2 * pad);

    // Vòng nền (track)
    QPen trackPen(Qt::lightGray, 8, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(trackPen);
    painter.drawArc(circleRect, 0, 360 * 16);

    // Vòng tiến trình
    QPen progressPen(Qt::darkBlue, 8, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progressPen);
    int spanAngle = static_cast<int>(360.0 * m_percent / 100.0 * 16);
    // Bắt đầu từ đỉnh (12h), vẽ theo chiều kim đồng hồ
    painter.drawArc(circleRect, 90 * 16, -spanAngle);

    // Text phần trăm ở giữa
    painter.setPen(Qt::black);
    QFont f = painter.font();
    f.setBold(true);
    f.setPointSize(14);
    painter.setFont(f);
    painter.drawText(rect(), Qt::AlignCenter, QString("%1%").arg(m_percent));
}