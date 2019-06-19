#ifndef WINDOWSWIDGET_H
#define WINDOWSWIDGET_H

#include "appwindow.h"
#include "ui_windowswidget.h"

/*************************************************************************************************************************/
class WindowsWidget : public QWidget
{
    Q_OBJECT

public:
    enum WindowAction { Update, Keep, Discard };

    struct WindowInfo {
        HWND hwnd;
        QString executableFilePath;
        QDateTime lastSeen;
        Qt::WindowState windowState;
        QString windowTitle;
        QRect geometry;
        bool visible;

        QList<QTableWidgetItem *> tableWidgetItems;
        bool mark;

        bool sameAs(const WindowInfo &other) const;
        WindowAction recommendedAction() const;
        void update(const WindowInfo &other, QTableWidget *tableWidget, bool &changedFlag);
        void forget();
    };

    WindowsWidget(const AppWindow::ScreenConfiguration &screenConfiguration, AppWindow *parent);

    AppWindow::ScreenConfiguration screenConfiguration() const { return m_screenConfiguration; }

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
    const AppWindow::ScreenConfiguration m_screenConfiguration;

    QList<WindowInfo> m_tempWindowsInfo;
    QList<WindowInfo> m_windows;
};

Q_DECLARE_METATYPE(WindowsWidget::WindowInfo);

#endif
