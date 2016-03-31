#ifndef TSK_DATADISPATCH_H
#define TSK_DATADISPATCH_H

#include <QThread>
#include "struct.h"
#include "constant.h"
#include "mainwindow.h"

class tsk_dataDispatch : public QThread
{
    Q_OBJECT
public:
    explicit tsk_dataDispatch(QObject *parent = 0);
    void stop();
    void allocaLinkedBuf(_tagBufList **pBufHead, int bufNum, int maxWait_ms);
    int    filledLinkedBuf(_tagBufList *pBufHead, char bufHead[]);
public:
   class MainWindow *m_pParent;
protected:
    void run();
private:
    volatile bool stopped;
signals:
    
public slots:
    
};

#endif // TSK_DATADISPATCH_H
