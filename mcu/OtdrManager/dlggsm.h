#ifndef DLGGSM_H
#define DLGGSM_H

#include "mainwindow.h"
#include "struct.h"
#include <QDialog>

namespace Ui {
class dlgGsm;
}

class dlgGsm : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgGsm(QWidget *parent = 0,int frameNo = 0, int cardNo = 0,\
                    int devType = GSM);
    ~dlgGsm();
public:
    //指向父窗口的类
    MainWindow * m_pParent;
    //从机号,槽位号，设备类型
    _tagDlgAttribute dlgAttribute;
public:
    void LayoutOnDlg();    
private slots:
    void on_pushBtnGetCardVerson_clicked();


    void on_pushBtnExit_clicked();

private:
    Ui::dlgGsm *ui;
};

#endif // DLGGSM_H
