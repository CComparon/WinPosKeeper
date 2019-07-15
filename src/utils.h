#ifndef WINPOSKEEPER_UTILS_H
#define WINPOSKEEPER_UTILS_H

/*************************************************************************************************************************/
struct ScreenConfiguration : public QString
{
    static ScreenConfiguration current();
    static QString toString(QScreen *screen);
    bool isValid() const { return !isEmpty(); }
};

/*************************************************************************************************************************/
struct WindowInfoBase
{
    HWND hwnd;
    QString executableFilePath;
    Qt::WindowState windowState;
    QString windowTitle;
    QRect geometry;
    bool visible;

    static bool fromWinId(HWND winId, WindowInfoBase *info = nullptr);
};

#endif
