/****************************************************************************
** Meta object code from reading C++ file 'tsk_sms_send.h'
**
** Created: Thu Dec 17 10:22:09 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../OtdrManager/tsk_sms_send.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tsk_sms_send.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_tsk_SMS_Send[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_tsk_SMS_Send[] = {
    "tsk_SMS_Send\0"
};

const QMetaObject tsk_SMS_Send::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_tsk_SMS_Send,
      qt_meta_data_tsk_SMS_Send, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &tsk_SMS_Send::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *tsk_SMS_Send::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *tsk_SMS_Send::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tsk_SMS_Send))
        return static_cast<void*>(const_cast< tsk_SMS_Send*>(this));
    return QThread::qt_metacast(_clname);
}

int tsk_SMS_Send::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
