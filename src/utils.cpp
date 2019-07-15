#include "utils.h"

/*************************************************************************************************************************/
ScreenConfiguration ScreenConfiguration::current()
{
    QStringList tmp;
    for (QScreen *screen : qApp->screens())
        tmp << toString(screen);
    ScreenConfiguration ret;
    ret.append(tmp.join(" "));
    return ret;
}

QString ScreenConfiguration::toString(QScreen *screen)
{
    QString ret;
    QDebug(&ret) << screen->geometry();
    if (screen == qApp->primaryScreen())
        ret = QSL("PRIMARY(%1)").arg(ret.trimmed().replace("QRect", "Rect"));
    return ret;
}

/*************************************************************************************************************************/
bool WindowInfoBase::fromWinId(HWND winId, WindowInfoBase *info)
{
    wchar_t buf1[100000];
    if (!::GetWindowTextW(winId, buf1, sizeof(buf1)/sizeof(*buf1)))
        return false;

    RECT rect;
    if (!::GetWindowRect(winId, &rect))
        return false;

    DWORD processId = 0;
    DWORD ret = ::GetWindowThreadProcessId(winId, &processId);
    if (!ret || !processId)
        return false;

    HANDLE processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processId);
    wchar_t buf2[100000];
    if (!::GetModuleFileNameExW(processHandle, nullptr, buf2, sizeof(buf2)/sizeof(*buf2)))
        return false;

    WINDOWPLACEMENT windowPlacement;
    if (!::GetWindowPlacement(winId, &windowPlacement))
        return false;

    if (info)
        *info = WindowInfoBase{
            winId,
            QString::fromWCharArray(buf2),
            windowPlacement.showCmd == SW_SHOWNORMAL ? Qt::WindowNoState :
                    (windowPlacement.showCmd == SW_SHOWMAXIMIZED ? Qt::WindowMaximized : Qt::WindowMinimized),
            QString::fromWCharArray(buf1),
            QRect(QPoint(rect.left, rect.top), QPoint(rect.right, rect.bottom)),
            !!::IsWindowVisible(winId)
        };

    return true;
}
