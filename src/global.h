#ifndef WINDOWSPOSKEEPER_GLOBAL_H
#define WINDOWSPOSKEEPER_GLOBAL_H

#include <QtCore>
#include <QtWidgets>

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
