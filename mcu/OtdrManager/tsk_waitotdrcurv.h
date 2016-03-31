#ifndef TSK_WAITOTDRCURV_H
#define TSK_WAITOTDRCURV_H

#include <QThread>
#include "tsk_otdrmanage.h"
class tsk_waitOtdrCurv : public QThread
{
    Q_OBJECT
public:
    explicit tsk_waitOtdrCurv(QObject *parent = 0);
public:
    void run();
    void stop();
public:
    class tsk_OtdrManage *pOtdrManager;
private:
    volatile bool stopped;
    
signals:
    
public slots:
    
};

#endif // TSK_WAITOTDRCURV_H
