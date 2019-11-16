/****************************************************************************
** Meta object code from reading C++ file 'spotshapes.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../spotshapes.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'spotshapes.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SpotShapeStar_t {
    QByteArrayData data[8];
    char stringdata0[86];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SpotShapeStar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SpotShapeStar_t qt_meta_stringdata_SpotShapeStar = {
    {
QT_MOC_LITERAL(0, 0, 13), // "SpotShapeStar"
QT_MOC_LITERAL(1, 14, 12), // "colorChanged"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 5), // "color"
QT_MOC_LITERAL(4, 34, 13), // "pointsChanged"
QT_MOC_LITERAL(5, 48, 6), // "points"
QT_MOC_LITERAL(6, 55, 18), // "innerRadiusChanged"
QT_MOC_LITERAL(7, 74, 11) // "innerRadius"

    },
    "SpotShapeStar\0colorChanged\0\0color\0"
    "pointsChanged\0points\0innerRadiusChanged\0"
    "innerRadius"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SpotShapeStar[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       3,   38, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    1,   32,    2, 0x06 /* Public */,
       6,    1,   35,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QColor,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    7,

 // properties: name, type, flags
       3, QMetaType::QColor, 0x00495103,
       5, QMetaType::Int, 0x00495103,
       7, QMetaType::Int, 0x00495103,

 // properties: notify_signal_id
       0,
       1,
       2,

       0        // eod
};

void SpotShapeStar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SpotShapeStar *_t = static_cast<SpotShapeStar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->colorChanged((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        case 1: _t->pointsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->innerRadiusChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SpotShapeStar::*)(QColor );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SpotShapeStar::colorChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SpotShapeStar::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SpotShapeStar::pointsChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SpotShapeStar::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SpotShapeStar::innerRadiusChanged)) {
                *result = 2;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        SpotShapeStar *_t = static_cast<SpotShapeStar *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->color(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->points(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->innerRadius(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        SpotShapeStar *_t = static_cast<SpotShapeStar *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setColor(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->setPoints(*reinterpret_cast< int*>(_v)); break;
        case 2: _t->setInnerRadius(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject SpotShapeStar::staticMetaObject = {
    { &QQuickItem::staticMetaObject, qt_meta_stringdata_SpotShapeStar.data,
      qt_meta_data_SpotShapeStar,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SpotShapeStar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SpotShapeStar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SpotShapeStar.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int SpotShapeStar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void SpotShapeStar::colorChanged(QColor _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SpotShapeStar::pointsChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SpotShapeStar::innerRadiusChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_SpotShapeNGon_t {
    QByteArrayData data[6];
    char stringdata0[53];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SpotShapeNGon_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SpotShapeNGon_t qt_meta_stringdata_SpotShapeNGon = {
    {
QT_MOC_LITERAL(0, 0, 13), // "SpotShapeNGon"
QT_MOC_LITERAL(1, 14, 12), // "colorChanged"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 5), // "color"
QT_MOC_LITERAL(4, 34, 12), // "sidesChanged"
QT_MOC_LITERAL(5, 47, 5) // "sides"

    },
    "SpotShapeNGon\0colorChanged\0\0color\0"
    "sidesChanged\0sides"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SpotShapeNGon[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       2,   30, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QColor,    3,
    QMetaType::Void, QMetaType::Int,    5,

 // properties: name, type, flags
       3, QMetaType::QColor, 0x00495103,
       5, QMetaType::Int, 0x00495103,

 // properties: notify_signal_id
       0,
       1,

       0        // eod
};

void SpotShapeNGon::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SpotShapeNGon *_t = static_cast<SpotShapeNGon *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->colorChanged((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        case 1: _t->sidesChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SpotShapeNGon::*)(QColor );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SpotShapeNGon::colorChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SpotShapeNGon::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SpotShapeNGon::sidesChanged)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        SpotShapeNGon *_t = static_cast<SpotShapeNGon *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->color(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->sides(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        SpotShapeNGon *_t = static_cast<SpotShapeNGon *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setColor(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->setSides(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject SpotShapeNGon::staticMetaObject = {
    { &QQuickItem::staticMetaObject, qt_meta_stringdata_SpotShapeNGon.data,
      qt_meta_data_SpotShapeNGon,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SpotShapeNGon::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SpotShapeNGon::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SpotShapeNGon.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int SpotShapeNGon::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void SpotShapeNGon::colorChanged(QColor _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SpotShapeNGon::sidesChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
