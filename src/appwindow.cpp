#include "appwindow.h"
#include "utils.h"
#include "windowswidget.h"
#include "windowpicker.h"

/*************************************************************************************************************************/
AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.textEditLogs->document()->setMaximumBlockCount(100);
    ui.windowsWidgets->clear();

    m_autoCaptureTimer.setSingleShot(false);
    m_autoCaptureTimer.start();
    connect(&m_autoCaptureTimer, &QTimer::timeout, this, &AppWindow::capture);
    on_sliderAutoCapturePeriod_valueChanged(ui.sliderAutoCapturePeriod->value());

    m_systemTrayIcon.setIcon(qApp->windowIcon());
    m_systemTrayIcon.setToolTip(qApp->applicationName());
    QMenu *trayMenu = new QMenu(this);
    trayMenu->addSection("Actions");
    m_captureAction = trayMenu->addAction(ui.btnCaptureNow->text(), this, &AppWindow::on_btnCaptureNow_clicked);
    m_restoreAction = trayMenu->addAction(ui.btnRestore->text(), this, &AppWindow::on_btnRestore_clicked);
    m_restoreAction->setEnabled(false);
    trayMenu->addSection("Views");
    trayMenu->addAction("Open simple view", this, [this]() { this->openView(false); });
    trayMenu->addAction("Open advanced view", this, [this]() { this->openView(true); });
    trayMenu->addSeparator();
    trayMenu->addAction("&Quit", qApp, &QCoreApplication::quit);
    m_systemTrayIcon.setContextMenu(trayMenu);
    connect(&m_systemTrayIcon, &QSystemTrayIcon::activated, this, &AppWindow::onSystemTrayActivated);
    m_systemTrayIcon.setVisible(true); // necessary to force private signal-signal connections

    connect(qApp, &QApplication::screenAdded, this, &AppWindow::onScreenAdded);
    connect(qApp, &QApplication::screenRemoved, this, &AppWindow::onScreenRemoved);
    connect(qApp, &QApplication::primaryScreenChanged, this, &AppWindow::onPrimaryScreenChanged);

    for (QScreen *screen : qApp->screens())
        onScreenAdded(screen);

    log(QSL("Current screen configuration: %1").arg(ScreenConfiguration::current()));
    updateUi();
}

void AppWindow::log(const QString &line)
{
    QString html = QSL("<b>%1</b> %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", " ")).arg(line);
    ui.textEditLogs->append(html);
}

void AppWindow::log(const QStringList &lines)
{
    log(lines.join("<br/>"));
}

void AppWindow::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide();
}

/*************************************************************************************************************************/
WindowsWidget *AppWindow::getOrCreateWindowsWidget(const ScreenConfiguration &screenConfiguration_)
{
    const ScreenConfiguration &screenConfiguration = screenConfiguration_.isValid() ? screenConfiguration_ : ScreenConfiguration::current();

    if (!m_windowsWidgets.contains(screenConfiguration)) {
        WindowsWidget *windowsWidget = new WindowsWidget(screenConfiguration, this);
        ui.windowsWidgets->addTab(windowsWidget, screenConfiguration);
        m_windowsWidgets[screenConfiguration] = windowsWidget;
    }

    return m_windowsWidgets[screenConfiguration];
}

void AppWindow::discardWindowsWidget(const ScreenConfiguration &screenConfiguration_)
{
    const ScreenConfiguration &screenConfiguration = screenConfiguration_.isValid() ? screenConfiguration_ : ScreenConfiguration::current();

    if (m_windowsWidgets.contains(screenConfiguration)) {
        m_windowsWidgets[screenConfiguration]->deleteLater();
        m_windowsWidgets.remove(screenConfiguration);
    }
}

/*************************************************************************************************************************/
void AppWindow::openView(bool advanced)
{
    if (m_advancedView != advanced) {
        m_advancedView = advanced;
        updateUi();
    }
    showNormal();
    QTimer::singleShot(0, this, &QWidget::raise);
    QTimer::singleShot(0, this, &QWidget::activateWindow);
}

/*************************************************************************************************************************/
void AppWindow::on_sliderAutoCapturePeriod_valueChanged(int value)
{
    static const QVector<QPair<int, QString>> intervals{ { 0, "never"}, { 1, "1 second"}, { 10, "10 seconds"}, { 60, "1 minute"}, { 600, "10 minutes"}, { 3600, "1 hour"}, { 3600*4, "4 hours"} };
    value = qBound(0, value, intervals.count());
    m_autoCaptureTimer.setInterval(intervals[value].first * 1000);
    if (m_autoCaptureTimer.isActive() && intervals[value].first <= 0)
        m_autoCaptureTimer.stop();
    else if (!m_autoCaptureTimer.isActive() && intervals[value].first > 0)
        m_autoCaptureTimer.start();
    ui.lblAutoCapturePeriod->setText(intervals[value].second);
}

void AppWindow::on_btnCaptureNow_clicked()
{
    capture();
}

void AppWindow::on_btnRestore_clicked()
{
    restore();
}

void AppWindow::on_windowsWidgets_tabCloseRequested(int tabIndex)
{
    WindowsWidget *windowsWidget = qobject_cast<WindowsWidget *>(ui.windowsWidgets->widget(tabIndex));
    if (windowsWidget)
        discardWindowsWidget(windowsWidget->screenConfiguration());
}

void AppWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
        openView(m_advancedView);
}

void AppWindow::onScreenAdded(QScreen *screen)
{
    log(QStringList { QSL("Screen added: %1").arg(ScreenConfiguration::toString(screen)),
                      QSL("Screen configuration is now: %1").arg(ScreenConfiguration::current()) });
    connect(screen, &QScreen::geometryChanged, this, &AppWindow::onScreenGeometryChanged, Qt::UniqueConnection);
    updateUi();
}

void AppWindow::onScreenRemoved(QScreen *screen)
{
    log(QStringList { QSL("Screen removed: %1").arg(ScreenConfiguration::toString(screen)),
                      QSL("Screen configuration is now: %1").arg(ScreenConfiguration::current()) });
    updateUi();
}

void AppWindow::onPrimaryScreenChanged(QScreen *screen)
{
    log(QStringList { QSL("Primary screen changed."),
                      QSL("Screen configuration is now: %1").arg(ScreenConfiguration::current()) });
    updateUi();
}

void AppWindow::onScreenGeometryChanged(const QRect &geometry)
{
    QScreen *screen = qobject_cast<QScreen *>(sender());
    log(QStringList { QSL("A Screen geometry changed to %1").arg(ScreenConfiguration::toString(screen)),
                      QSL("Screen configuration is now: %1").arg(ScreenConfiguration::current()) });
    updateUi();
}

void AppWindow::updateUi()
{
    ui.textEditLogs->setVisible(m_advancedView);
    ui.btnClearLogs->setVisible(m_advancedView);

    bool foundCurrent = false;
    const ScreenConfiguration screenConfiguration = ScreenConfiguration::current();
    for (int i=0; i<ui.windowsWidgets->count(); i++) {
        auto *windowsWidget = qobject_cast<WindowsWidget *>(ui.windowsWidgets->widget(i));
        bool current = windowsWidget->screenConfiguration() == screenConfiguration;
        foundCurrent = foundCurrent || current;
        static const QIcon currentTabIcon(":/appicon.png");
        ui.windowsWidgets->setTabIcon(i, current ? currentTabIcon : QIcon());
    }
    m_restoreAction->setEnabled(foundCurrent);
}

void AppWindow::capture()
{
    log(QSL("Capturing for screen configuration: %1").arg(ScreenConfiguration::current()));
    getOrCreateWindowsWidget()->capture();
    updateUi();
}

void AppWindow::restore()
{
    log(QSL("Restoring for screen configuration: %1").arg(ScreenConfiguration::current()));
    getOrCreateWindowsWidget()->restore();
    updateUi();
}
