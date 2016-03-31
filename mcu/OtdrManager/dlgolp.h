#ifndef DLGOLP_H
#define DLGOLP_H

#include <QDialog>
#include "mainwindow.h"
#include "struct.h"
namespace Ui {
class dlgOlp;
}

class dlgOlp : public QDialog
{
    Q_OBJECT
    
public:
    explicit dlgOlp(QWidget *parent = 0,int frameNo = 0, int cardNo = 0,\
                    int devType = OLP);
    ~dlgOlp();
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
    Ui::dlgOlp *ui;
};

#endif // DLGOLP_H
