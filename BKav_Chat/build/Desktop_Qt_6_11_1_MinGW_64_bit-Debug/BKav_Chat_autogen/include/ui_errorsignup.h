/********************************************************************************
** Form generated from reading UI file 'errorsignup.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ERRORSIGNUP_H
#define UI_ERRORSIGNUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ErrorSignUp
{
public:

    void setupUi(QWidget *ErrorSignUp)
    {
        if (ErrorSignUp->objectName().isEmpty())
            ErrorSignUp->setObjectName("ErrorSignUp");
        ErrorSignUp->resize(400, 300);

        retranslateUi(ErrorSignUp);

        QMetaObject::connectSlotsByName(ErrorSignUp);
    } // setupUi

    void retranslateUi(QWidget *ErrorSignUp)
    {
        ErrorSignUp->setWindowTitle(QCoreApplication::translate("ErrorSignUp", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ErrorSignUp: public Ui_ErrorSignUp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ERRORSIGNUP_H
