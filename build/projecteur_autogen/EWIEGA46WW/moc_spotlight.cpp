/****************************************************************************
** Meta object code from reading C++ file 'spotlight.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../spotlight.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'spotlight.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Spotlight_t {
    QByteArrayData data[10];
    char stringdata0[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Spotlight_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Spotlight_t qt_meta_stringdata_Spotlight = {
    {
QT_MOC_LITERAL(0, 0, 9), // "Spotlight"
QT_MOC_LITERAL(1, 10, 5), // "error"
QT_MOC_LITERAL(2, 16, 0), // ""
QT_MOC_LITERAL(3, 17, 6), // "errMsg"
QT_MOC_LITERAL(4, 24, 9), // "connected"
QT_MOC_LITERAL(5, 34, 10), // "devicePath"
QT_MOC_LITERAL(6, 45, 12), // "disconnected"
QT_MOC_LITERAL(7, 58, 34), // "anySpotlightDeviceConnectedCh..."
QT_MOC_LITERAL(8, 93, 17), // "spotActiveChanged"
QT_MOC_LITERAL(9, 111, 8) // "isActive"

    },
    "Spotlight\0error\0\0errMsg\0connected\0"
    "devicePath\0disconnected\0"
    "anySpotlightDeviceConnectedChanged\0"
    "spotActiveChanged\0isActive"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Spotlight[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    1,   42,    2, 0x06 /* Public */,
       6,    1,   45,    2, 0x06 /* Public */,
       7,    1,   48,    2, 0x06 /* Public */,
       8,    1,   51,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    9,

       0        // eod
};

void Spotlight::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Spotlight *_t = static_cast<Spotlight *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->connected((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->disconnected((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->anySpotlightDeviceConnectedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->spotActiveChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Spotlight::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Spotlight::error)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Spotlight::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Spotlight::connected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Spotlight::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Spotlight::disconnected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Spotlight::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Spotlight::anySpotlightDeviceConnectedChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Spotlight::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Spotlight::spotActiveChanged)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Spotlight::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Spotlight.data,
      qt_meta_data_Spotlight,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Spotlight::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Spotlight::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Spotlight.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Spotlight::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void Spotlight::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Spotlight::connected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Spotlight::disconnected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Spotlight::anySpotlightDeviceConnectedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Spotlight::spotActiveChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
