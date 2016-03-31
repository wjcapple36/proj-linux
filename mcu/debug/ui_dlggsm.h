/********************************************************************************
** Form generated from reading UI file 'dlggsm.ui'
**
** Created: Thu Mar 31 08:52:21 2016
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGGSM_H
#define UI_DLGGSM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_dlgGsm
{
public:
    QTabWidget *tabWdgtCard;
    QWidget *tabCardVerson;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineEdtFrame;
    QLabel *label_2;
    QLineEdit *lineEdtCard;
    QLabel *label_3;
    QLineEdit *lineEdtDev;
    QLabel *label_4;
    QLineEdit *lineEdtVerson;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushBtnGetCardVerson;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushBtnExit;
    QSpacerItem *horizontalSpacer_2;
    QWidget *tab_2;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayoutMain;

    void setupUi(QDialog *dlgGsm)
    {
        if (dlgGsm->objectName().isEmpty())
            dlgGsm->setObjectName(QString::fromUtf8("dlgGsm"));
        dlgGsm->resize(888, 813);
        QFont font;
        font.setFamily(QString::fromUtf8("AR PL UKai CN"));
        font.setPointSize(20);
        dlgGsm->setFont(font);
        tabWdgtCard = new QTabWidget(dlgGsm);
        tabWdgtCard->setObjectName(QString::fromUtf8("tabWdgtCard"));
        tabWdgtCard->setGeometry(QRect(140, 50, 651, 681));
        tabCardVerson = new QWidget();
        tabCardVerson->setObjectName(QString::fromUtf8("tabCardVerson"));
        verticalLayout = new QVBoxLayout(tabCardVerson);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setLabelAlignment(Qt::AlignCenter);
        formLayout->setFormAlignment(Qt::AlignCenter);
        label = new QLabel(tabCardVerson);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(150, 35));
        label->setMaximumSize(QSize(150, 16777215));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        lineEdtFrame = new QLineEdit(tabCardVerson);
        lineEdtFrame->setObjectName(QString::fromUtf8("lineEdtFrame"));
        lineEdtFrame->setMinimumSize(QSize(150, 35));
        lineEdtFrame->setMaximumSize(QSize(150, 35));
        lineEdtFrame->setReadOnly(true);

        formLayout->setWidget(0, QFormLayout::FieldRole, lineEdtFrame);

        label_2 = new QLabel(tabCardVerson);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(150, 35));
        label_2->setMaximumSize(QSize(150, 16777215));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        lineEdtCard = new QLineEdit(tabCardVerson);
        lineEdtCard->setObjectName(QString::fromUtf8("lineEdtCard"));
        lineEdtCard->setMinimumSize(QSize(150, 35));
        lineEdtCard->setMaximumSize(QSize(150, 35));
        lineEdtCard->setReadOnly(true);

        formLayout->setWidget(1, QFormLayout::FieldRole, lineEdtCard);

        label_3 = new QLabel(tabCardVerson);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(150, 35));
        label_3->setMaximumSize(QSize(150, 35));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_3);

        lineEdtDev = new QLineEdit(tabCardVerson);
        lineEdtDev->setObjectName(QString::fromUtf8("lineEdtDev"));
        lineEdtDev->setMinimumSize(QSize(150, 35));
        lineEdtDev->setMaximumSize(QSize(150, 35));
        lineEdtDev->setReadOnly(true);

        formLayout->setWidget(2, QFormLayout::FieldRole, lineEdtDev);

        label_4 = new QLabel(tabCardVerson);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(150, 35));
        label_4->setMaximumSize(QSize(150, 35));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_4);

        lineEdtVerson = new QLineEdit(tabCardVerson);
        lineEdtVerson->setObjectName(QString::fromUtf8("lineEdtVerson"));
        lineEdtVerson->setMinimumSize(QSize(150, 35));
        lineEdtVerson->setMaximumSize(QSize(150, 35));
        lineEdtVerson->setReadOnly(true);

        formLayout->setWidget(3, QFormLayout::FieldRole, lineEdtVerson);


        verticalLayout->addLayout(formLayout);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushBtnGetCardVerson = new QPushButton(tabCardVerson);
        pushBtnGetCardVerson->setObjectName(QString::fromUtf8("pushBtnGetCardVerson"));
        pushBtnGetCardVerson->setMinimumSize(QSize(280, 35));
        pushBtnGetCardVerson->setMaximumSize(QSize(280, 35));

        horizontalLayout->addWidget(pushBtnGetCardVerson);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        pushBtnExit = new QPushButton(tabCardVerson);
        pushBtnExit->setObjectName(QString::fromUtf8("pushBtnExit"));
        pushBtnExit->setMinimumSize(QSize(280, 35));
        pushBtnExit->setMaximumSize(QSize(280, 35));

        horizontalLayout->addWidget(pushBtnExit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout);

        tabWdgtCard->addTab(tabCardVerson, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        tabWdgtCard->addTab(tab_2, QString());
        gridLayoutWidget = new QWidget(dlgGsm);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(129, 80, 211, 91));
        gridLayoutMain = new QGridLayout(gridLayoutWidget);
        gridLayoutMain->setObjectName(QString::fromUtf8("gridLayoutMain"));
        gridLayoutMain->setContentsMargins(0, 0, 0, 0);

        retranslateUi(dlgGsm);

        tabWdgtCard->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(dlgGsm);
    } // setupUi

    void retranslateUi(QDialog *dlgGsm)
    {
        dlgGsm->setWindowTitle(QApplication::translate("dlgGsm", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("dlgGsm", "\346\234\272\346\241\206", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("dlgGsm", "\346\247\275\344\275\215", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("dlgGsm", "\350\256\276\345\244\207\347\261\273\345\236\213", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("dlgGsm", "\350\275\257\344\273\266\347\211\210\346\234\254", 0, QApplication::UnicodeUTF8));
        pushBtnGetCardVerson->setText(QApplication::translate("dlgGsm", "\350\216\267\345\217\226\346\235\277\345\215\241\347\211\210\346\234\254", 0, QApplication::UnicodeUTF8));
        pushBtnExit->setText(QApplication::translate("dlgGsm", "\351\200\200\345\207\272", 0, QApplication::UnicodeUTF8));
        tabWdgtCard->setTabText(tabWdgtCard->indexOf(tabCardVerson), QApplication::translate("dlgGsm", "\347\211\210\346\234\254\345\217\267", 0, QApplication::UnicodeUTF8));
        tabWdgtCard->setTabText(tabWdgtCard->indexOf(tab_2), QApplication::translate("dlgGsm", "Tab 2", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class dlgGsm: public Ui_dlgGsm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGGSM_H
