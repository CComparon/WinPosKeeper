#ifndef WINPOSKEEPER_APPWINDOW_H
#define WINPOSKEEPER_APPWINDOW_H

#include "ui_appwindow.h"
#include "utils.h"

class WindowsWidget;

/*************************************************************************************************************************/
class AppWindow : public QWidget
{
    Q_OBJECT

public:
    AppWindow(QWidget *parent = nullptr);

    void log(const QString &line);
    void log(const QStringList &lines);

private:
    void closeEvent(QCloseEvent *);

    WindowsWidget *getOrCreateWindowsWidget(const ScreenConfiguration &screenConfiguration = ScreenConfiguration());
    void discardWindowsWidget(const ScreenConfiguration &screenConfiguration = ScreenConfiguration());

private slots:
    void openView(bool advanced);

    void on_sliderAutoCapturePeriod_valueChanged(int value);
    void on_btnCaptureNow_clicked();
    void on_btnRestore_clicked();
    void on_windowsWidgets_tabCloseRequested(int);

    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);

    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void onPrimaryScreenChanged(QScreen *screen);
    void onScreenGeometryChanged(const QRect &geometry);

    void updateUi();

    void capture();
    void restore();

private:
    Ui::AppWindow ui;
    bool m_advancedView =
#ifdef QT_DEBUG
            true;
#else
            false;
#endif

    QAction *m_captureAction;
    QAction *m_restoreAction;
    QSystemTrayIcon m_systemTrayIcon;

    QTimer m_autoCaptureTimer;

    QHash<ScreenConfiguration, WindowsWidget *> m_windowsWidgets;
};

#endif
