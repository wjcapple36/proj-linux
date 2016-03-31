/********************************************************************************
** Form generated from reading UI file 'dlgmcu.ui'
**
** Created: Thu Mar 31 08:52:21 2016
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGMCU_H
#define UI_DLGMCU_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QDlgMcu
{
public:
    QWidget *wdgtAddFrame;
    QVBoxLayout *verticalLayout;
    QTableWidget *listFrame;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *hLayout;
    QSpacerItem *horizontalSpacer_2;
    QComboBox *combBoxAddFrame;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushBtnAddFrame;
    QSpacerItem *horizontalSpacer_10;
    QPushButton *pushBtnDelFrame;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushBtnExit;
    QSpacerItem *horizontalSpacer_9;
    QSpacerItem *verticalSpacer_3;
    QWidget *wdgtDisplay;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_5;
    QLabel *labelShowFrame;
    QSpacerItem *horizontalSpacer_4;
    QLabel *labelSoftV;
    QSpacerItem *horizontalSpacer_11;
    QSpacerItem *verticalSpacer_2;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_7;
    QComboBox *combBoxShowFram;
    QSpacerItem *horizontalSpacer_6;
    QPushButton *pushBtnShowFram;
    QSpacerItem *horizontalSpacer_8;
    QWidget *wdgtModifySlot;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLineEdit *lineEdtSubrack;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_6;
    QLabel *label_5;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *labelSlot_1;
    QComboBox *comboxSlot_1;
    QComboBox *comBoxSlotPort_1;
    QHBoxLayout *horizontalLayout_6;
    QLabel *labelSlot_2;
    QComboBox *comboxSlot_2;
    QComboBox *comBoxSlotPort_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *labelSlot_3;
    QComboBox *comboxSlot_3;
    QComboBox *comBoxSlotPort_3;
    QHBoxLayout *horizontalLayout_8;
    QLabel *labelSlot_4;
    QComboBox *comboxSlot_4;
    QComboBox *comBoxSlotPort_4;
    QHBoxLayout *horizontalLayout_9;
    QLabel *labelSlot_5;
    QComboBox *comboxSlot_5;
    QComboBox *comBoxSlotPort_5;
    QHBoxLayout *horizontalLayout_10;
    QLabel *labelSlot_6;
    QComboBox *comboxSlot_6;
    QComboBox *comBoxSlotPort_6;
    QHBoxLayout *horizontalLayout_11;
    QLabel *labelSlot_7;
    QComboBox *comboxSlot_7;
    QComboBox *comBoxSlotPort_7;
    QHBoxLayout *horizontalLayout_12;
    QLabel *labelSlot_8;
    QComboBox *comboxSlot_8;
    QComboBox *comBoxSlotPort_8;
    QHBoxLayout *horizontalLayout_13;
    QLabel *labelSlot_9;
    QComboBox *comboxSlot_9;
    QComboBox *comBoxSlotPort_9;
    QHBoxLayout *horizontalLayout_14;
    QLabel *labelSlot_10;
    QComboBox *comboxSlot_10;
    QComboBox *comBoxSlotPort_10;
    QHBoxLayout *horizontalLayout_15;
    QLabel *labelSlot_11;
    QComboBox *comboxSlot_11;
    QComboBox *comBoxSlotPort_11;
    QHBoxLayout *horizontalLayout_16;
    QLabel *labelSlot_12;
    QComboBox *comboxSlot_12;
    QComboBox *comBoxSlotPort_12;
    QHBoxLayout *horizontalLayout_18;
    QLabel *labelSlot_13;
    QComboBox *comboxSlot_13;
    QComboBox *comBoxSlotPort_13;
    QHBoxLayout *horizontalLayout_19;
    QLabel *labelSlot_14;
    QComboBox *comboxSlot_14;
    QComboBox *comBoxSlotPort_14;
    QHBoxLayout *horizontalLayout_20;
    QLabel *labelSlot_15;
    QComboBox *comboxSlot_15;
    QComboBox *comBoxSlotPort_15;
    QHBoxLayout *horizontalLayout_21;
    QLabel *labelSlot_16;
    QComboBox *comboxSlot_16;
    QComboBox *comBoxSlotPort_16;
    QSpacerItem *verticalSpacer_4;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushBtnViewCurCfg;
    QPushButton *pushBtnModifyFram;
    QPushButton *pushBtnSaveFram;
    QSpacerItem *verticalSpacer_5;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayoutMain;

    void setupUi(QDialog *QDlgMcu)
    {
        if (QDlgMcu->objectName().isEmpty())
            QDlgMcu->setObjectName(QString::fromUtf8("QDlgMcu"));
        QDlgMcu->resize(1250, 1044);
        QFont font;
        font.setFamily(QString::fromUtf8("AR PL UKai CN"));
        font.setPointSize(20);
        QDlgMcu->setFont(font);
        wdgtAddFrame = new QWidget(QDlgMcu);
        wdgtAddFrame->setObjectName(QString::fromUtf8("wdgtAddFrame"));
        wdgtAddFrame->setGeometry(QRect(0, 660, 511, 281));
        verticalLayout = new QVBoxLayout(wdgtAddFrame);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        listFrame = new QTableWidget(wdgtAddFrame);
        listFrame->setObjectName(QString::fromUtf8("listFrame"));

        verticalLayout->addWidget(listFrame);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);

        hLayout = new QHBoxLayout();
        hLayout->setObjectName(QString::fromUtf8("hLayout"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayout->addItem(horizontalSpacer_2);

        combBoxAddFrame = new QComboBox(wdgtAddFrame);
        combBoxAddFrame->setObjectName(QString::fromUtf8("combBoxAddFrame"));
        combBoxAddFrame->setMinimumSize(QSize(150, 35));
        combBoxAddFrame->setMaximumSize(QSize(150, 35));
        QFont font1;
        font1.setPointSize(24);
        combBoxAddFrame->setFont(font1);

        hLayout->addWidget(combBoxAddFrame);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayout->addItem(horizontalSpacer);

        pushBtnAddFrame = new QPushButton(wdgtAddFrame);
        pushBtnAddFrame->setObjectName(QString::fromUtf8("pushBtnAddFrame"));
        pushBtnAddFrame->setMinimumSize(QSize(150, 35));
        pushBtnAddFrame->setMaximumSize(QSize(150, 35));

        hLayout->addWidget(pushBtnAddFrame);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayout->addItem(horizontalSpacer_10);

        pushBtnDelFrame = new QPushButton(wdgtAddFrame);
        pushBtnDelFrame->setObjectName(QString::fromUtf8("pushBtnDelFrame"));
        pushBtnDelFrame->setMinimumSize(QSize(150, 35));
        pushBtnDelFrame->setMaximumSize(QSize(150, 35));

        hLayout->addWidget(pushBtnDelFrame);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayout->addItem(horizontalSpacer_3);

        pushBtnExit = new QPushButton(wdgtAddFrame);
        pushBtnExit->setObjectName(QString::fromUtf8("pushBtnExit"));
        pushBtnExit->setMinimumSize(QSize(150, 35));
        pushBtnExit->setMaximumSize(QSize(150, 35));

        hLayout->addWidget(pushBtnExit);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayout->addItem(horizontalSpacer_9);


        verticalLayout->addLayout(hLayout);

        verticalSpacer_3 = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        wdgtDisplay = new QWidget(QDlgMcu);
        wdgtDisplay->setObjectName(QString::fromUtf8("wdgtDisplay"));
        wdgtDisplay->setGeometry(QRect(10, 130, 391, 381));
        verticalLayout_2 = new QVBoxLayout(wdgtDisplay);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_5);

        labelShowFrame = new QLabel(wdgtDisplay);
        labelShowFrame->setObjectName(QString::fromUtf8("labelShowFrame"));
        labelShowFrame->setMinimumSize(QSize(260, 35));
        labelShowFrame->setMaximumSize(QSize(260, 35));

        horizontalLayout->addWidget(labelShowFrame);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        labelSoftV = new QLabel(wdgtDisplay);
        labelSoftV->setObjectName(QString::fromUtf8("labelSoftV"));
        labelSoftV->setMinimumSize(QSize(320, 35));
        labelSoftV->setMaximumSize(QSize(320, 35));

        horizontalLayout->addWidget(labelSoftV);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_11);


        verticalLayout_2->addLayout(horizontalLayout);

        verticalSpacer_2 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_2->addItem(verticalSpacer_2);

        groupBox = new QGroupBox(wdgtDisplay);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_4 = new QHBoxLayout(groupBox);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_7);

        combBoxShowFram = new QComboBox(groupBox);
        combBoxShowFram->setObjectName(QString::fromUtf8("combBoxShowFram"));
        combBoxShowFram->setMinimumSize(QSize(150, 35));
        combBoxShowFram->setMaximumSize(QSize(150, 35));
        combBoxShowFram->setFont(font1);

        horizontalLayout_4->addWidget(combBoxShowFram);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_6);

        pushBtnShowFram = new QPushButton(groupBox);
        pushBtnShowFram->setObjectName(QString::fromUtf8("pushBtnShowFram"));
        pushBtnShowFram->setMinimumSize(QSize(150, 35));
        pushBtnShowFram->setMaximumSize(QSize(150, 35));

        horizontalLayout_4->addWidget(pushBtnShowFram);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_8);


        verticalLayout_2->addWidget(groupBox);

        wdgtModifySlot = new QWidget(QDlgMcu);
        wdgtModifySlot->setObjectName(QString::fromUtf8("wdgtModifySlot"));
        wdgtModifySlot->setGeometry(QRect(490, 30, 821, 821));
        verticalLayout_3 = new QVBoxLayout(wdgtModifySlot);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(wdgtModifySlot);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(70, 35));
        label_2->setMaximumSize(QSize(70, 35));

        horizontalLayout_3->addWidget(label_2);

        lineEdtSubrack = new QLineEdit(wdgtModifySlot);
        lineEdtSubrack->setObjectName(QString::fromUtf8("lineEdtSubrack"));
        lineEdtSubrack->setMinimumSize(QSize(300, 35));
        lineEdtSubrack->setMaximumSize(QSize(300, 35));
        lineEdtSubrack->setFont(font1);
        lineEdtSubrack->setReadOnly(true);

        horizontalLayout_3->addWidget(lineEdtSubrack);


        verticalLayout_3->addLayout(horizontalLayout_3);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_6 = new QLabel(wdgtModifySlot);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setMinimumSize(QSize(70, 35));
        label_6->setMaximumSize(QSize(70, 35));

        horizontalLayout_17->addWidget(label_6);

        label_5 = new QLabel(wdgtModifySlot);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(150, 35));
        label_5->setMaximumSize(QSize(150, 35));

        horizontalLayout_17->addWidget(label_5);

        label_4 = new QLabel(wdgtModifySlot);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(120, 35));
        label_4->setMaximumSize(QSize(120, 35));
        QFont font2;
        font2.setPointSize(20);
        label_4->setFont(font2);

        horizontalLayout_17->addWidget(label_4);


        verticalLayout_3->addLayout(horizontalLayout_17);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        labelSlot_1 = new QLabel(wdgtModifySlot);
        labelSlot_1->setObjectName(QString::fromUtf8("labelSlot_1"));
        labelSlot_1->setMinimumSize(QSize(70, 35));
        labelSlot_1->setMaximumSize(QSize(70, 35));

        horizontalLayout_5->addWidget(labelSlot_1);

        comboxSlot_1 = new QComboBox(wdgtModifySlot);
        comboxSlot_1->setObjectName(QString::fromUtf8("comboxSlot_1"));
        comboxSlot_1->setMinimumSize(QSize(150, 35));
        comboxSlot_1->setMaximumSize(QSize(150, 35));
        comboxSlot_1->setFont(font1);

        horizontalLayout_5->addWidget(comboxSlot_1);

        comBoxSlotPort_1 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_1->setObjectName(QString::fromUtf8("comBoxSlotPort_1"));
        comBoxSlotPort_1->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_1->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_1->setFont(font1);

        horizontalLayout_5->addWidget(comBoxSlotPort_1);


        verticalLayout_3->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        labelSlot_2 = new QLabel(wdgtModifySlot);
        labelSlot_2->setObjectName(QString::fromUtf8("labelSlot_2"));
        labelSlot_2->setMinimumSize(QSize(70, 35));
        labelSlot_2->setMaximumSize(QSize(70, 35));

        horizontalLayout_6->addWidget(labelSlot_2);

        comboxSlot_2 = new QComboBox(wdgtModifySlot);
        comboxSlot_2->setObjectName(QString::fromUtf8("comboxSlot_2"));
        comboxSlot_2->setMinimumSize(QSize(150, 35));
        comboxSlot_2->setMaximumSize(QSize(150, 35));
        comboxSlot_2->setFont(font1);

        horizontalLayout_6->addWidget(comboxSlot_2);

        comBoxSlotPort_2 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_2->setObjectName(QString::fromUtf8("comBoxSlotPort_2"));
        comBoxSlotPort_2->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_2->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_2->setFont(font1);

        horizontalLayout_6->addWidget(comBoxSlotPort_2);


        verticalLayout_3->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        labelSlot_3 = new QLabel(wdgtModifySlot);
        labelSlot_3->setObjectName(QString::fromUtf8("labelSlot_3"));
        labelSlot_3->setMinimumSize(QSize(70, 35));
        labelSlot_3->setMaximumSize(QSize(70, 35));

        horizontalLayout_7->addWidget(labelSlot_3);

        comboxSlot_3 = new QComboBox(wdgtModifySlot);
        comboxSlot_3->setObjectName(QString::fromUtf8("comboxSlot_3"));
        comboxSlot_3->setMinimumSize(QSize(150, 35));
        comboxSlot_3->setMaximumSize(QSize(150, 35));
        comboxSlot_3->setFont(font1);

        horizontalLayout_7->addWidget(comboxSlot_3);

        comBoxSlotPort_3 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_3->setObjectName(QString::fromUtf8("comBoxSlotPort_3"));
        comBoxSlotPort_3->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_3->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_3->setFont(font1);

        horizontalLayout_7->addWidget(comBoxSlotPort_3);


        verticalLayout_3->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        labelSlot_4 = new QLabel(wdgtModifySlot);
        labelSlot_4->setObjectName(QString::fromUtf8("labelSlot_4"));
        labelSlot_4->setMinimumSize(QSize(70, 35));
        labelSlot_4->setMaximumSize(QSize(70, 35));

        horizontalLayout_8->addWidget(labelSlot_4);

        comboxSlot_4 = new QComboBox(wdgtModifySlot);
        comboxSlot_4->setObjectName(QString::fromUtf8("comboxSlot_4"));
        comboxSlot_4->setMinimumSize(QSize(150, 35));
        comboxSlot_4->setMaximumSize(QSize(150, 35));
        comboxSlot_4->setFont(font1);

        horizontalLayout_8->addWidget(comboxSlot_4);

        comBoxSlotPort_4 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_4->setObjectName(QString::fromUtf8("comBoxSlotPort_4"));
        comBoxSlotPort_4->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_4->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_4->setFont(font1);

        horizontalLayout_8->addWidget(comBoxSlotPort_4);


        verticalLayout_3->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        labelSlot_5 = new QLabel(wdgtModifySlot);
        labelSlot_5->setObjectName(QString::fromUtf8("labelSlot_5"));
        labelSlot_5->setMinimumSize(QSize(70, 35));
        labelSlot_5->setMaximumSize(QSize(70, 35));

        horizontalLayout_9->addWidget(labelSlot_5);

        comboxSlot_5 = new QComboBox(wdgtModifySlot);
        comboxSlot_5->setObjectName(QString::fromUtf8("comboxSlot_5"));
        comboxSlot_5->setMinimumSize(QSize(150, 35));
        comboxSlot_5->setMaximumSize(QSize(150, 35));
        comboxSlot_5->setFont(font1);

        horizontalLayout_9->addWidget(comboxSlot_5);

        comBoxSlotPort_5 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_5->setObjectName(QString::fromUtf8("comBoxSlotPort_5"));
        comBoxSlotPort_5->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_5->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_5->setFont(font1);

        horizontalLayout_9->addWidget(comBoxSlotPort_5);


        verticalLayout_3->addLayout(horizontalLayout_9);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        labelSlot_6 = new QLabel(wdgtModifySlot);
        labelSlot_6->setObjectName(QString::fromUtf8("labelSlot_6"));
        labelSlot_6->setMinimumSize(QSize(70, 35));
        labelSlot_6->setMaximumSize(QSize(70, 35));

        horizontalLayout_10->addWidget(labelSlot_6);

        comboxSlot_6 = new QComboBox(wdgtModifySlot);
        comboxSlot_6->setObjectName(QString::fromUtf8("comboxSlot_6"));
        comboxSlot_6->setMinimumSize(QSize(150, 35));
        comboxSlot_6->setMaximumSize(QSize(150, 35));
        comboxSlot_6->setFont(font1);

        horizontalLayout_10->addWidget(comboxSlot_6);

        comBoxSlotPort_6 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_6->setObjectName(QString::fromUtf8("comBoxSlotPort_6"));
        comBoxSlotPort_6->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_6->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_6->setFont(font1);

        horizontalLayout_10->addWidget(comBoxSlotPort_6);


        verticalLayout_3->addLayout(horizontalLayout_10);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        labelSlot_7 = new QLabel(wdgtModifySlot);
        labelSlot_7->setObjectName(QString::fromUtf8("labelSlot_7"));
        labelSlot_7->setMinimumSize(QSize(70, 35));
        labelSlot_7->setMaximumSize(QSize(70, 35));

        horizontalLayout_11->addWidget(labelSlot_7);

        comboxSlot_7 = new QComboBox(wdgtModifySlot);
        comboxSlot_7->setObjectName(QString::fromUtf8("comboxSlot_7"));
        comboxSlot_7->setMinimumSize(QSize(150, 35));
        comboxSlot_7->setMaximumSize(QSize(150, 35));
        comboxSlot_7->setFont(font1);

        horizontalLayout_11->addWidget(comboxSlot_7);

        comBoxSlotPort_7 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_7->setObjectName(QString::fromUtf8("comBoxSlotPort_7"));
        comBoxSlotPort_7->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_7->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_7->setFont(font1);

        horizontalLayout_11->addWidget(comBoxSlotPort_7);


        verticalLayout_3->addLayout(horizontalLayout_11);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        labelSlot_8 = new QLabel(wdgtModifySlot);
        labelSlot_8->setObjectName(QString::fromUtf8("labelSlot_8"));
        labelSlot_8->setMinimumSize(QSize(70, 35));
        labelSlot_8->setMaximumSize(QSize(70, 35));

        horizontalLayout_12->addWidget(labelSlot_8);

        comboxSlot_8 = new QComboBox(wdgtModifySlot);
        comboxSlot_8->setObjectName(QString::fromUtf8("comboxSlot_8"));
        comboxSlot_8->setMinimumSize(QSize(150, 35));
        comboxSlot_8->setMaximumSize(QSize(150, 35));
        comboxSlot_8->setFont(font1);

        horizontalLayout_12->addWidget(comboxSlot_8);

        comBoxSlotPort_8 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_8->setObjectName(QString::fromUtf8("comBoxSlotPort_8"));
        comBoxSlotPort_8->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_8->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_8->setFont(font1);

        horizontalLayout_12->addWidget(comBoxSlotPort_8);


        verticalLayout_3->addLayout(horizontalLayout_12);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        labelSlot_9 = new QLabel(wdgtModifySlot);
        labelSlot_9->setObjectName(QString::fromUtf8("labelSlot_9"));
        labelSlot_9->setMinimumSize(QSize(70, 35));
        labelSlot_9->setMaximumSize(QSize(70, 35));

        horizontalLayout_13->addWidget(labelSlot_9);

        comboxSlot_9 = new QComboBox(wdgtModifySlot);
        comboxSlot_9->setObjectName(QString::fromUtf8("comboxSlot_9"));
        comboxSlot_9->setMinimumSize(QSize(150, 35));
        comboxSlot_9->setMaximumSize(QSize(150, 35));
        comboxSlot_9->setFont(font1);

        horizontalLayout_13->addWidget(comboxSlot_9);

        comBoxSlotPort_9 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_9->setObjectName(QString::fromUtf8("comBoxSlotPort_9"));
        comBoxSlotPort_9->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_9->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_9->setFont(font1);

        horizontalLayout_13->addWidget(comBoxSlotPort_9);


        verticalLayout_3->addLayout(horizontalLayout_13);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        labelSlot_10 = new QLabel(wdgtModifySlot);
        labelSlot_10->setObjectName(QString::fromUtf8("labelSlot_10"));
        labelSlot_10->setMinimumSize(QSize(70, 35));
        labelSlot_10->setMaximumSize(QSize(70, 35));

        horizontalLayout_14->addWidget(labelSlot_10);

        comboxSlot_10 = new QComboBox(wdgtModifySlot);
        comboxSlot_10->setObjectName(QString::fromUtf8("comboxSlot_10"));
        comboxSlot_10->setMinimumSize(QSize(150, 35));
        comboxSlot_10->setMaximumSize(QSize(150, 35));
        comboxSlot_10->setFont(font1);

        horizontalLayout_14->addWidget(comboxSlot_10);

        comBoxSlotPort_10 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_10->setObjectName(QString::fromUtf8("comBoxSlotPort_10"));
        comBoxSlotPort_10->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_10->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_10->setFont(font1);

        horizontalLayout_14->addWidget(comBoxSlotPort_10);


        verticalLayout_3->addLayout(horizontalLayout_14);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        labelSlot_11 = new QLabel(wdgtModifySlot);
        labelSlot_11->setObjectName(QString::fromUtf8("labelSlot_11"));
        labelSlot_11->setMinimumSize(QSize(70, 35));
        labelSlot_11->setMaximumSize(QSize(70, 35));

        horizontalLayout_15->addWidget(labelSlot_11);

        comboxSlot_11 = new QComboBox(wdgtModifySlot);
        comboxSlot_11->setObjectName(QString::fromUtf8("comboxSlot_11"));
        comboxSlot_11->setMinimumSize(QSize(150, 35));
        comboxSlot_11->setMaximumSize(QSize(150, 35));
        comboxSlot_11->setFont(font1);

        horizontalLayout_15->addWidget(comboxSlot_11);

        comBoxSlotPort_11 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_11->setObjectName(QString::fromUtf8("comBoxSlotPort_11"));
        comBoxSlotPort_11->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_11->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_11->setFont(font1);

        horizontalLayout_15->addWidget(comBoxSlotPort_11);


        verticalLayout_3->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        labelSlot_12 = new QLabel(wdgtModifySlot);
        labelSlot_12->setObjectName(QString::fromUtf8("labelSlot_12"));
        labelSlot_12->setMinimumSize(QSize(70, 35));
        labelSlot_12->setMaximumSize(QSize(70, 35));

        horizontalLayout_16->addWidget(labelSlot_12);

        comboxSlot_12 = new QComboBox(wdgtModifySlot);
        comboxSlot_12->setObjectName(QString::fromUtf8("comboxSlot_12"));
        comboxSlot_12->setMinimumSize(QSize(150, 35));
        comboxSlot_12->setMaximumSize(QSize(150, 35));
        comboxSlot_12->setFont(font1);

        horizontalLayout_16->addWidget(comboxSlot_12);

        comBoxSlotPort_12 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_12->setObjectName(QString::fromUtf8("comBoxSlotPort_12"));
        comBoxSlotPort_12->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_12->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_12->setFont(font1);

        horizontalLayout_16->addWidget(comBoxSlotPort_12);


        verticalLayout_3->addLayout(horizontalLayout_16);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        labelSlot_13 = new QLabel(wdgtModifySlot);
        labelSlot_13->setObjectName(QString::fromUtf8("labelSlot_13"));
        labelSlot_13->setMinimumSize(QSize(70, 35));
        labelSlot_13->setMaximumSize(QSize(70, 35));

        horizontalLayout_18->addWidget(labelSlot_13);

        comboxSlot_13 = new QComboBox(wdgtModifySlot);
        comboxSlot_13->setObjectName(QString::fromUtf8("comboxSlot_13"));
        comboxSlot_13->setMinimumSize(QSize(150, 35));
        comboxSlot_13->setMaximumSize(QSize(150, 35));
        comboxSlot_13->setFont(font1);

        horizontalLayout_18->addWidget(comboxSlot_13);

        comBoxSlotPort_13 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_13->setObjectName(QString::fromUtf8("comBoxSlotPort_13"));
        comBoxSlotPort_13->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_13->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_13->setFont(font1);

        horizontalLayout_18->addWidget(comBoxSlotPort_13);


        verticalLayout_3->addLayout(horizontalLayout_18);

        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        labelSlot_14 = new QLabel(wdgtModifySlot);
        labelSlot_14->setObjectName(QString::fromUtf8("labelSlot_14"));
        labelSlot_14->setMinimumSize(QSize(70, 35));
        labelSlot_14->setMaximumSize(QSize(70, 35));

        horizontalLayout_19->addWidget(labelSlot_14);

        comboxSlot_14 = new QComboBox(wdgtModifySlot);
        comboxSlot_14->setObjectName(QString::fromUtf8("comboxSlot_14"));
        comboxSlot_14->setMinimumSize(QSize(150, 35));
        comboxSlot_14->setMaximumSize(QSize(150, 35));
        comboxSlot_14->setFont(font1);

        horizontalLayout_19->addWidget(comboxSlot_14);

        comBoxSlotPort_14 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_14->setObjectName(QString::fromUtf8("comBoxSlotPort_14"));
        comBoxSlotPort_14->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_14->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_14->setFont(font1);

        horizontalLayout_19->addWidget(comBoxSlotPort_14);


        verticalLayout_3->addLayout(horizontalLayout_19);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        labelSlot_15 = new QLabel(wdgtModifySlot);
        labelSlot_15->setObjectName(QString::fromUtf8("labelSlot_15"));
        labelSlot_15->setMinimumSize(QSize(70, 35));
        labelSlot_15->setMaximumSize(QSize(70, 35));

        horizontalLayout_20->addWidget(labelSlot_15);

        comboxSlot_15 = new QComboBox(wdgtModifySlot);
        comboxSlot_15->setObjectName(QString::fromUtf8("comboxSlot_15"));
        comboxSlot_15->setMinimumSize(QSize(150, 35));
        comboxSlot_15->setMaximumSize(QSize(150, 35));
        comboxSlot_15->setFont(font1);

        horizontalLayout_20->addWidget(comboxSlot_15);

        comBoxSlotPort_15 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_15->setObjectName(QString::fromUtf8("comBoxSlotPort_15"));
        comBoxSlotPort_15->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_15->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_15->setFont(font1);

        horizontalLayout_20->addWidget(comBoxSlotPort_15);


        verticalLayout_3->addLayout(horizontalLayout_20);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName(QString::fromUtf8("horizontalLayout_21"));
        labelSlot_16 = new QLabel(wdgtModifySlot);
        labelSlot_16->setObjectName(QString::fromUtf8("labelSlot_16"));
        labelSlot_16->setMinimumSize(QSize(70, 35));
        labelSlot_16->setMaximumSize(QSize(70, 35));

        horizontalLayout_21->addWidget(labelSlot_16);

        comboxSlot_16 = new QComboBox(wdgtModifySlot);
        comboxSlot_16->setObjectName(QString::fromUtf8("comboxSlot_16"));
        comboxSlot_16->setMinimumSize(QSize(150, 35));
        comboxSlot_16->setMaximumSize(QSize(150, 35));
        comboxSlot_16->setFont(font1);

        horizontalLayout_21->addWidget(comboxSlot_16);

        comBoxSlotPort_16 = new QComboBox(wdgtModifySlot);
        comBoxSlotPort_16->setObjectName(QString::fromUtf8("comBoxSlotPort_16"));
        comBoxSlotPort_16->setMinimumSize(QSize(120, 35));
        comBoxSlotPort_16->setMaximumSize(QSize(120, 35));
        comBoxSlotPort_16->setFont(font1);

        horizontalLayout_21->addWidget(comBoxSlotPort_16);


        verticalLayout_3->addLayout(horizontalLayout_21);

        verticalSpacer_4 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushBtnViewCurCfg = new QPushButton(wdgtModifySlot);
        pushBtnViewCurCfg->setObjectName(QString::fromUtf8("pushBtnViewCurCfg"));
        pushBtnViewCurCfg->setMinimumSize(QSize(120, 35));
        pushBtnViewCurCfg->setMaximumSize(QSize(120, 35));

        horizontalLayout_2->addWidget(pushBtnViewCurCfg);

        pushBtnModifyFram = new QPushButton(wdgtModifySlot);
        pushBtnModifyFram->setObjectName(QString::fromUtf8("pushBtnModifyFram"));
        pushBtnModifyFram->setMinimumSize(QSize(120, 35));
        pushBtnModifyFram->setMaximumSize(QSize(120, 35));

        horizontalLayout_2->addWidget(pushBtnModifyFram);

        pushBtnSaveFram = new QPushButton(wdgtModifySlot);
        pushBtnSaveFram->setObjectName(QString::fromUtf8("pushBtnSaveFram"));
        pushBtnSaveFram->setMinimumSize(QSize(120, 35));
        pushBtnSaveFram->setMaximumSize(QSize(120, 35));

        horizontalLayout_2->addWidget(pushBtnSaveFram);


        verticalLayout_3->addLayout(horizontalLayout_2);

        verticalSpacer_5 = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_5);

        gridLayoutWidget = new QWidget(QDlgMcu);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(210, 500, 41, 91));
        gridLayoutMain = new QGridLayout(gridLayoutWidget);
        gridLayoutMain->setObjectName(QString::fromUtf8("gridLayoutMain"));
        gridLayoutMain->setContentsMargins(0, 0, 0, 0);

        retranslateUi(QDlgMcu);

        QMetaObject::connectSlotsByName(QDlgMcu);
    } // setupUi

    void retranslateUi(QDialog *QDlgMcu)
    {
        QDlgMcu->setWindowTitle(QApplication::translate("QDlgMcu", "Dialog", 0, QApplication::UnicodeUTF8));
        pushBtnAddFrame->setText(QApplication::translate("QDlgMcu", "\345\242\236\345\212\240\344\273\216\346\234\272", 0, QApplication::UnicodeUTF8));
        pushBtnDelFrame->setText(QApplication::translate("QDlgMcu", "\345\210\240\351\231\244\344\273\216\346\234\272", 0, QApplication::UnicodeUTF8));
        pushBtnExit->setText(QApplication::translate("QDlgMcu", "\351\200\200\345\207\272", 0, QApplication::UnicodeUTF8));
        labelShowFrame->setText(QApplication::translate("QDlgMcu", "\346\230\276\347\244\272\346\234\272\346\241\206\347\274\226\345\217\267", 0, QApplication::UnicodeUTF8));
        labelSoftV->setText(QApplication::translate("QDlgMcu", "\350\275\257\344\273\266\347\211\210\346\234\254\345\217\267:", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("QDlgMcu", "\345\210\207\346\215\242\346\230\276\347\244\272\346\234\272\346\241\206", 0, QApplication::UnicodeUTF8));
        pushBtnShowFram->setText(QApplication::translate("QDlgMcu", "\345\210\207\346\215\242\346\230\276\347\244\272\346\234\272\346\241\206", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("QDlgMcu", "\344\273\216\346\234\272\345\217\267", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\215\345\217\267", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("QDlgMcu", "\347\261\273\345\236\213", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("QDlgMcu", "\347\253\257\345\217\243\346\225\260\347\233\256", 0, QApplication::UnicodeUTF8));
        labelSlot_1->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2151", 0, QApplication::UnicodeUTF8));
        labelSlot_2->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2152", 0, QApplication::UnicodeUTF8));
        labelSlot_3->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2153", 0, QApplication::UnicodeUTF8));
        labelSlot_4->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2154", 0, QApplication::UnicodeUTF8));
        labelSlot_5->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2155", 0, QApplication::UnicodeUTF8));
        labelSlot_6->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2156", 0, QApplication::UnicodeUTF8));
        labelSlot_7->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2157", 0, QApplication::UnicodeUTF8));
        labelSlot_8->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2158", 0, QApplication::UnicodeUTF8));
        labelSlot_9->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\2159", 0, QApplication::UnicodeUTF8));
        labelSlot_10->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21510", 0, QApplication::UnicodeUTF8));
        labelSlot_11->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21511", 0, QApplication::UnicodeUTF8));
        labelSlot_12->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21512", 0, QApplication::UnicodeUTF8));
        labelSlot_13->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21513", 0, QApplication::UnicodeUTF8));
        labelSlot_14->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21514", 0, QApplication::UnicodeUTF8));
        labelSlot_15->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21515", 0, QApplication::UnicodeUTF8));
        labelSlot_16->setText(QApplication::translate("QDlgMcu", "\346\247\275\344\275\21516", 0, QApplication::UnicodeUTF8));
        pushBtnViewCurCfg->setText(QApplication::translate("QDlgMcu", "\346\237\245\347\234\213\345\256\236\346\227\266\347\261\273\345\236\213", 0, QApplication::UnicodeUTF8));
        pushBtnModifyFram->setText(QApplication::translate("QDlgMcu", "\344\275\277\347\224\250\345\275\223\345\211\215\351\205\215\347\275\256", 0, QApplication::UnicodeUTF8));
        pushBtnSaveFram->setText(QApplication::translate("QDlgMcu", "\344\277\235\345\255\230\345\205\250\351\203\250\351\205\215\347\275\256", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class QDlgMcu: public Ui_QDlgMcu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGMCU_H
