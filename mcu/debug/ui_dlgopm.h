/********************************************************************************
** Form generated from reading UI file 'dlgopm.ui'
**
** Created: Thu Mar 31 08:52:21 2016
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGOPM_H
#define UI_DLGOPM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
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

class Ui_dlgOpm
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayoutMain;
    QTabWidget *tabWdgtCard;
    QWidget *tabPower;
    QWidget *layoutWidget;
    QGridLayout *gridLayoutPower;
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
    QGroupBox *groupBoxTitle;
    QGroupBox *groupBoxPw;
    QGroupBox *groupBoxOperate;
    QWidget *widgetOperate;
    QWidget *widgetDeviceInfo;
    QWidget *layoutWidget1;
    QVBoxLayout *VHLayoutOperateBtn;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *HLayoutOperateBtn;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *pushBbtnGetPw;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pushBtnStopGet;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *pushBtnExit;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *verticalSpacer_3;
    QWidget *layoutWidget2;
    QVBoxLayout *VLayoutDeviceInfo;
    QSpacerItem *verticalSpacer_4;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_7;
    QFormLayout *formLayout_2;
    QLabel *label_5;
    QLineEdit *lineEdtFrame_1;
    QLabel *label_6;
    QLineEdit *lineEdtCard_1;
    QSpacerItem *horizontalSpacer_8;
    QFormLayout *formLayout_3;
    QLabel *label_9;
    QLineEdit *lineEdtType_1;
    QLineEdit *lineEdtPort_1;
    QLabel *label_10;
    QSpacerItem *horizontalSpacer_9;
    QSpacerItem *verticalSpacer_5;

    void setupUi(QDialog *dlgOpm)
    {
        if (dlgOpm->objectName().isEmpty())
            dlgOpm->setObjectName(QString::fromUtf8("dlgOpm"));
        dlgOpm->resize(938, 878);
        QFont font;
        font.setFamily(QString::fromUtf8("AR PL UKai CN"));
        font.setPointSize(20);
        dlgOpm->setFont(font);
        gridLayoutWidget = new QWidget(dlgOpm);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(80, -10, 211, 91));
        gridLayoutMain = new QGridLayout(gridLayoutWidget);
        gridLayoutMain->setObjectName(QString::fromUtf8("gridLayoutMain"));
        gridLayoutMain->setContentsMargins(0, 0, 0, 0);
        tabWdgtCard = new QTabWidget(dlgOpm);
        tabWdgtCard->setObjectName(QString::fromUtf8("tabWdgtCard"));
        tabWdgtCard->setGeometry(QRect(70, 30, 541, 411));
        tabPower = new QWidget();
        tabPower->setObjectName(QString::fromUtf8("tabPower"));
        layoutWidget = new QWidget(tabPower);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(9, 9, 211, 151));
        gridLayoutPower = new QGridLayout(layoutWidget);
        gridLayoutPower->setObjectName(QString::fromUtf8("gridLayoutPower"));
        gridLayoutPower->setContentsMargins(0, 0, 0, 0);
        tabWdgtCard->addTab(tabPower, QString());
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
        lineEdtFrame->setEnabled(true);
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
        lineEdtCard->setEnabled(true);
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
        lineEdtDev->setEnabled(true);
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
        lineEdtVerson->setEnabled(true);
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


        verticalLayout->addLayout(horizontalLayout);

        tabWdgtCard->addTab(tabCardVerson, QString());
        groupBoxTitle = new QGroupBox(dlgOpm);
        groupBoxTitle->setObjectName(QString::fromUtf8("groupBoxTitle"));
        groupBoxTitle->setGeometry(QRect(490, 220, 201, 111));
        groupBoxPw = new QGroupBox(dlgOpm);
        groupBoxPw->setObjectName(QString::fromUtf8("groupBoxPw"));
        groupBoxPw->setGeometry(QRect(430, 90, 168, 80));
        groupBoxOperate = new QGroupBox(dlgOpm);
        groupBoxOperate->setObjectName(QString::fromUtf8("groupBoxOperate"));
        groupBoxOperate->setGeometry(QRect(620, 610, 168, 80));
        widgetOperate = new QWidget(dlgOpm);
        widgetOperate->setObjectName(QString::fromUtf8("widgetOperate"));
        widgetOperate->setGeometry(QRect(70, 470, 81, 41));
        widgetDeviceInfo = new QWidget(dlgOpm);
        widgetDeviceInfo->setObjectName(QString::fromUtf8("widgetDeviceInfo"));
        widgetDeviceInfo->setGeometry(QRect(30, 780, 120, 80));
        layoutWidget1 = new QWidget(dlgOpm);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(160, 450, 490, 81));
        VHLayoutOperateBtn = new QVBoxLayout(layoutWidget1);
        VHLayoutOperateBtn->setObjectName(QString::fromUtf8("VHLayoutOperateBtn"));
        VHLayoutOperateBtn->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        VHLayoutOperateBtn->addItem(verticalSpacer_2);

        HLayoutOperateBtn = new QHBoxLayout();
        HLayoutOperateBtn->setObjectName(QString::fromUtf8("HLayoutOperateBtn"));
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        HLayoutOperateBtn->addItem(horizontalSpacer_5);

        pushBbtnGetPw = new QPushButton(layoutWidget1);
        pushBbtnGetPw->setObjectName(QString::fromUtf8("pushBbtnGetPw"));
        pushBbtnGetPw->setMinimumSize(QSize(150, 35));
        pushBbtnGetPw->setMaximumSize(QSize(150, 35));

        HLayoutOperateBtn->addWidget(pushBbtnGetPw);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        HLayoutOperateBtn->addItem(horizontalSpacer_2);

        pushBtnStopGet = new QPushButton(layoutWidget1);
        pushBtnStopGet->setObjectName(QString::fromUtf8("pushBtnStopGet"));
        pushBtnStopGet->setMinimumSize(QSize(150, 35));
        pushBtnStopGet->setMaximumSize(QSize(150, 35));

        HLayoutOperateBtn->addWidget(pushBtnStopGet);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        HLayoutOperateBtn->addItem(horizontalSpacer_4);

        pushBtnExit = new QPushButton(layoutWidget1);
        pushBtnExit->setObjectName(QString::fromUtf8("pushBtnExit"));
        pushBtnExit->setMinimumSize(QSize(150, 35));
        pushBtnExit->setMaximumSize(QSize(150, 35));

        HLayoutOperateBtn->addWidget(pushBtnExit);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        HLayoutOperateBtn->addItem(horizontalSpacer_6);


        VHLayoutOperateBtn->addLayout(HLayoutOperateBtn);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        VHLayoutOperateBtn->addItem(verticalSpacer_3);

        layoutWidget2 = new QWidget(dlgOpm);
        layoutWidget2->setObjectName(QString::fromUtf8("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(90, 580, 521, 181));
        VLayoutDeviceInfo = new QVBoxLayout(layoutWidget2);
        VLayoutDeviceInfo->setObjectName(QString::fromUtf8("VLayoutDeviceInfo"));
        VLayoutDeviceInfo->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        VLayoutDeviceInfo->addItem(verticalSpacer_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_7);

        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_5 = new QLabel(layoutWidget2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_5);

        lineEdtFrame_1 = new QLineEdit(layoutWidget2);
        lineEdtFrame_1->setObjectName(QString::fromUtf8("lineEdtFrame_1"));
        lineEdtFrame_1->setEnabled(true);
        lineEdtFrame_1->setReadOnly(true);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, lineEdtFrame_1);

        label_6 = new QLabel(layoutWidget2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_6);

        lineEdtCard_1 = new QLineEdit(layoutWidget2);
        lineEdtCard_1->setObjectName(QString::fromUtf8("lineEdtCard_1"));
        lineEdtCard_1->setEnabled(true);
        lineEdtCard_1->setReadOnly(true);

        formLayout_2->setWidget(1, QFormLayout::FieldRole, lineEdtCard_1);


        horizontalLayout_2->addLayout(formLayout_2);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_8);

        formLayout_3 = new QFormLayout();
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        label_9 = new QLabel(layoutWidget2);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_9);

        lineEdtType_1 = new QLineEdit(layoutWidget2);
        lineEdtType_1->setObjectName(QString::fromUtf8("lineEdtType_1"));
        lineEdtType_1->setEnabled(true);
        lineEdtType_1->setReadOnly(true);

        formLayout_3->setWidget(0, QFormLayout::FieldRole, lineEdtType_1);

        lineEdtPort_1 = new QLineEdit(layoutWidget2);
        lineEdtPort_1->setObjectName(QString::fromUtf8("lineEdtPort_1"));
        lineEdtPort_1->setEnabled(true);
        lineEdtPort_1->setReadOnly(true);

        formLayout_3->setWidget(1, QFormLayout::FieldRole, lineEdtPort_1);

        label_10 = new QLabel(layoutWidget2);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, label_10);


        horizontalLayout_2->addLayout(formLayout_3);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_9);


        VLayoutDeviceInfo->addLayout(horizontalLayout_2);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        VLayoutDeviceInfo->addItem(verticalSpacer_5);


        retranslateUi(dlgOpm);

        tabWdgtCard->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(dlgOpm);
    } // setupUi

    void retranslateUi(QDialog *dlgOpm)
    {
        dlgOpm->setWindowTitle(QApplication::translate("dlgOpm", "Dialog", 0, QApplication::UnicodeUTF8));
        tabWdgtCard->setTabText(tabWdgtCard->indexOf(tabPower), QApplication::translate("dlgOpm", "\345\275\223\345\211\215\345\212\237\347\216\207", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("dlgOpm", "\346\234\272\346\241\206", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("dlgOpm", "\346\247\275\344\275\215", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("dlgOpm", "\350\256\276\345\244\207\347\261\273\345\236\213", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("dlgOpm", "\350\275\257\344\273\266\347\211\210\346\234\254", 0, QApplication::UnicodeUTF8));
        pushBtnGetCardVerson->setText(QApplication::translate("dlgOpm", "\350\216\267\345\217\226\346\235\277\345\215\241\347\211\210\346\234\254", 0, QApplication::UnicodeUTF8));
        tabWdgtCard->setTabText(tabWdgtCard->indexOf(tabCardVerson), QApplication::translate("dlgOpm", "\347\211\210\346\234\254\345\217\267", 0, QApplication::UnicodeUTF8));
        groupBoxTitle->setTitle(QApplication::translate("dlgOpm", "\350\256\276\345\244\207\344\277\241\346\201\257", 0, QApplication::UnicodeUTF8));
        groupBoxPw->setTitle(QApplication::translate("dlgOpm", "\345\212\237\347\216\207(dB)", 0, QApplication::UnicodeUTF8));
        groupBoxOperate->setTitle(QApplication::translate("dlgOpm", "\346\223\215\344\275\234", 0, QApplication::UnicodeUTF8));
        pushBbtnGetPw->setText(QApplication::translate("dlgOpm", "\345\210\267\346\226\260\345\212\237\347\216\207", 0, QApplication::UnicodeUTF8));
        pushBtnStopGet->setText(QApplication::translate("dlgOpm", "\345\201\234\346\255\242\345\210\267\346\226\260", 0, QApplication::UnicodeUTF8));
        pushBtnExit->setText(QApplication::translate("dlgOpm", "\351\200\200\345\207\272", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("dlgOpm", "\346\234\272\346\241\206", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("dlgOpm", "\346\247\275\344\275\215", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("dlgOpm", "\347\261\273\345\236\213", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("dlgOpm", "\347\253\257\345\217\243\346\225\260\347\233\256", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class dlgOpm: public Ui_dlgOpm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGOPM_H
