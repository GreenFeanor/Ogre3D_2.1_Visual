/****************************************************************************
** Meta object code from reading C++ file 'QTOgreWindow.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../sources/QTOgreWindow.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QTOgreWindow.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QTOgreWindow_t {
    QByteArrayData data[11];
    char stringdata0[103];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QTOgreWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QTOgreWindow_t qt_meta_stringdata_QTOgreWindow = {
    {
QT_MOC_LITERAL(0, 0, 12), // "QTOgreWindow"
QT_MOC_LITERAL(1, 13, 14), // "entitySelected"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 11), // "Ogre::Item*"
QT_MOC_LITERAL(4, 41, 6), // "entity"
QT_MOC_LITERAL(5, 48, 11), // "renderLater"
QT_MOC_LITERAL(6, 60, 9), // "renderNow"
QT_MOC_LITERAL(7, 70, 11), // "eventFilter"
QT_MOC_LITERAL(8, 82, 6), // "target"
QT_MOC_LITERAL(9, 89, 7), // "QEvent*"
QT_MOC_LITERAL(10, 97, 5) // "event"

    },
    "QTOgreWindow\0entitySelected\0\0Ogre::Item*\0"
    "entity\0renderLater\0renderNow\0eventFilter\0"
    "target\0QEvent*\0event"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QTOgreWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   37,    2, 0x0a /* Public */,
       6,    0,   38,    2, 0x0a /* Public */,
       7,    2,   39,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QObjectStar, 0x80000000 | 9,    8,   10,

       0        // eod
};

void QTOgreWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QTOgreWindow *_t = static_cast<QTOgreWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->entitySelected((*reinterpret_cast< Ogre::Item*(*)>(_a[1]))); break;
        case 1: _t->renderLater(); break;
        case 2: _t->renderNow(); break;
        case 3: { bool _r = _t->eventFilter((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< QEvent*(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (QTOgreWindow::*_t)(Ogre::Item * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&QTOgreWindow::entitySelected)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject QTOgreWindow::staticMetaObject = {
    { &QWindow::staticMetaObject, qt_meta_stringdata_QTOgreWindow.data,
      qt_meta_data_QTOgreWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QTOgreWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QTOgreWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QTOgreWindow.stringdata0))
        return static_cast<void*>(const_cast< QTOgreWindow*>(this));
    if (!strcmp(_clname, "Ogre::FrameListener"))
        return static_cast< Ogre::FrameListener*>(const_cast< QTOgreWindow*>(this));
    if (!strcmp(_clname, "Ogre::RenderTargetListener"))
        return static_cast< Ogre::RenderTargetListener*>(const_cast< QTOgreWindow*>(this));
    return QWindow::qt_metacast(_clname);
}

int QTOgreWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QTOgreWindow::entitySelected(Ogre::Item * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
