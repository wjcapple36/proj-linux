/****************************************************************************
** Meta object code from reading C++ file 'tsk_ctuask.h'
**
** Created: Fri Nov 6 11:25:22 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OtdrManager/tsk_ctuask.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tsk_ctuask.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_tsk_ctuAsk[] = {

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
      16,   11,   12,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      32,   11,   12,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_tsk_ctuAsk[] = {
    "tsk_ctuAsk\0\0int\0findAlarm(uint)\0"
    "inspectFiber(uint)\0"
};

const QMetaObject tsk_ctuAsk::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_tsk_ctuAsk,
      qt_meta_data_tsk_ctuAsk, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &tsk_ctuAsk::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *tsk_ctuAsk::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *tsk_ctuAsk::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tsk_ctuAsk))
        return static_cast<void*>(const_cast< tsk_ctuAsk*>(this));
    return QThread::qt_metacast(_clname);
}

int tsk_ctuAsk::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
int tsk_ctuAsk::findAlarm(unsigned int _t1)
{
    int _t0;
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(&_t0)), const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
    return _t0;
}
QT_END_MOC_NAMESPACE