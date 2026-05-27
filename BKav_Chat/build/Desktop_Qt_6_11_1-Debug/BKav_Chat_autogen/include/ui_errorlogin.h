/********************************************************************************
** Form generated from reading UI file 'errorlogin.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ERRORLOGIN_H
#define UI_ERRORLOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ErrorLogIn
{
public:

    void setupUi(QWidget *ErrorLogIn)
    {
        if (ErrorLogIn->objectName().isEmpty())
            ErrorLogIn->setObjectName("ErrorLogIn");
        ErrorLogIn->resize(400, 300);

        retranslateUi(ErrorLogIn);

        QMetaObject::connectSlotsByName(ErrorLogIn);
    } // setupUi

    void retranslateUi(QWidget *ErrorLogIn)
    {
        ErrorLogIn->setWindowTitle(QCoreApplication::translate("ErrorLogIn", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ErrorLogIn: public Ui_ErrorLogIn {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ERRORLOGIN_H
