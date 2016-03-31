/****************************************************************************
** Meta object code from reading C++ file 'tsk_otdrmanage.h'
**
** Created: Thu Mar 31 11:48:25 2016
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OtdrManager/tsk_otdrmanage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tsk_otdrmanage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_tsk_OtdrManage[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   15,   16,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      36,   15,   16,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_tsk_OtdrManage[] = {
    "tsk_OtdrManage\0\0int\0findAlarm(uint)\0"
    "inspectFiber(uint)\0"
};

const QMetaObject tsk_OtdrManage::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_tsk_OtdrManage,
      qt_meta_data_tsk_OtdrManage, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &tsk_OtdrManage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *tsk_OtdrManage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *tsk_OtdrManage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tsk_OtdrManage))
        return static_cast<void*>(const_cast< tsk_OtdrManage*>(this));
    return QThread::qt_metacast(_clname);
}

int tsk_OtdrManage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { int _r = findAlarm((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 1: { int _r = inspectFiber((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
int tsk_OtdrManage::findAlarm(unsigned int _t1)
{
    int _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)), const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
    return _t0;
}
QT_END_MOC_NAMESPACE
