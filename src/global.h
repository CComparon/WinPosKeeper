#ifndef WINPOSKEEPER_GLOBAL_H
#define WINPOSKEEPER_GLOBAL_H

#include <QtCore>
#include <QtWidgets>
#include <qt_windows.h>
#include "windows.h"
#include "winuser.h"
#include "psapi.h"

#define QSL QStringLiteral
template<class T> using QSP = QSharedPointer<T>;
template<class T> using QWP = QWeakPointer<T>;

template <typename T> static QString qdebugToStringHelper(const T &val)
{
    QString ret;
    QDebug(&ret) << val;
    return ret;
}

#endif
