#ifndef DLGOSW_H
#define DLGOSW_H

#include <QDialog>
#include "mainwindow.h"
#include "struct.h"
namespace Ui {
class dlgOsw;
}

class dlgOsw : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgOsw(QWidget *parent = 0,int frameNo = 0, int cardNo = 0,\
                    int devType = OSW);
    ~dlgOsw();
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
    Ui::dlgOsw *ui;
};

#endif // DLGOSW_H
