/****************************************************************************
** Meta object code from reading C++ file 'tsk_datadispatch.h'
**
** Created: Thu Mar 31 11:48:28 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OtdrManager/tsk_datadispatch.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tsk_datadispatch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_tsk_dataDispatch[] = {

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

static const char qt_meta_stringdata_tsk_dataDispatch[] = {
    "tsk_dataDispatch\0"
};

const QMetaObject tsk_dataDispatch::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_tsk_dataDispatch,
      qt_meta_data_tsk_dataDispatch, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &tsk_dataDispatch::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *tsk_dataDispatch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *tsk_dataDispatch::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tsk_dataDispatch))
        return static_cast<void*>(const_cast< tsk_dataDispatch*>(this));
    return QThread::qt_metacast(_clname);
}

int tsk_dataDispatch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
