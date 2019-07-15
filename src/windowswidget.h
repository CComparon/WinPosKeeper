#ifndef WINDOWS_WIDGET_H
#define WINDOWS_WIDGET_H

#include "appwindow.h"
#include "ui_windowswidget.h"
#include "utils.h"
class AppWindow;

/*************************************************************************************************************************/
class WindowsWidget : public QWidget
{
    Q_OBJECT

public:
    enum WindowAction { Update, Keep, Discard };

    struct WindowInfo : public WindowInfoBase {
        WindowInfo() {}
        WindowInfo(const WindowInfoBase &base) : WindowInfoBase(base) {}

        QDateTime lastSeen = QDateTime::currentDateTime();
        QList<QTableWidgetItem *> tableWidgetItems;
        bool mark = false;

        bool sameAs(const WindowInfo &other) const;
        WindowAction recommendedAction() const;
        void update(const WindowInfo &other, QTableWidget *tableWidget, bool &changedFlag);
        void forget();
    };

    WindowsWidget(const ScreenConfiguration &screenConfiguration, AppWindow *parent);

    ScreenConfiguration screenConfiguration() const { return m_screenConfiguration; }

public slots:
    void capture();
    void restore(const WindowInfo *const windowInfo = nullptr);

private slots:
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

private:
    static int __stdcall enumWindowsCallback(HWND winId, LPARAM userParam);

    WindowInfo *findExistingWindowInfo(const WindowInfo &windowInfo);

private:
    Ui::WindowsWidget ui;
    AppWindow *const m_appWindow;
    const ScreenConfiguration m_screenConfiguration;

    QDateTime m_lastUpdateTime = QDateTime::currentDateTime();

    QList<WindowInfo> m_tempWindowsInfo;
    QList<WindowInfo> m_windows;
};

Q_DECLARE_METATYPE(WindowsWidget::WindowInfo);

#endif
