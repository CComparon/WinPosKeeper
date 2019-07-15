#include "windowswidget.h"

static void setTableWidgetItemTextHelper(QTableWidgetItem *item, const QString &text, bool &changedFlag)
{
    QFont fnt = item->font();
    if (item->text() == text) {
        fnt.setBold(false);
    } else {
        fnt.setBold(true);
        item->setText(text);
        changedFlag = true;
    }
    //item->setTextColor(QColor(qrand()%200, qrand()%200, qrand()%200));
    item->setFont(fnt);
}

/*************************************************************************************************************************/
bool WindowsWidget::WindowInfo::sameAs(const WindowInfo &other) const
{
    return hwnd == other.hwnd && executableFilePath == other.executableFilePath;
}

WindowsWidget::WindowAction WindowsWidget::WindowInfo::recommendedAction() const
{
    if (windowState == Qt::WindowMinimized)
        return Keep;
    if ((geometry.size().width() * geometry.size().height() < 1000)
        || geometry.left() == -32000
        || geometry.top() == -32000
        || windowTitle.trimmed().isEmpty()
        || executableFilePath.trimmed().isEmpty())
        return Discard;
    return Update;
}

void WindowsWidget::WindowInfo::update(const WindowInfo &other, QTableWidget *tableWidget, bool &changedFlag)
{
    Q_ASSERT(sameAs(other));
    const auto saved = tableWidgetItems;
    operator=(other);
    tableWidgetItems = saved;

    if (tableWidgetItems.isEmpty()) {
        tableWidget->insertRow(0);
        for (int j=0; j<tableWidget->columnCount(); j++) {
            QTableWidgetItem *item = new QTableWidgetItem;
            tableWidget->setItem(0, j, item);
            tableWidgetItems << item;
        }
    }

    tableWidgetItems[0]->setData(Qt::UserRole, QVariant::fromValue(*this));

    setTableWidgetItemTextHelper(tableWidgetItems[0], lastSeen.toString(Qt::ISODate).replace("T", " "), changedFlag);
    setTableWidgetItemTextHelper(tableWidgetItems[1], QString::number(quint64(hwnd)), changedFlag);
    setTableWidgetItemTextHelper(tableWidgetItems[2], QFileInfo(executableFilePath).fileName(), changedFlag);
    setTableWidgetItemTextHelper(tableWidgetItems[3], qdebugToStringHelper(geometry).replace("QRect(", "").replace(")", ""), changedFlag);
    setTableWidgetItemTextHelper(tableWidgetItems[4], windowState == Qt::WindowMinimized ? "minimized" : (windowState == Qt::WindowMaximized ? "maximized" : "normal"), changedFlag);
    setTableWidgetItemTextHelper(tableWidgetItems[5], windowTitle, changedFlag);
}

void WindowsWidget::WindowInfo::forget()
{
    if (!tableWidgetItems.isEmpty())
        tableWidgetItems.first()->tableWidget()->removeRow(tableWidgetItems.first()->row());
}

/*************************************************************************************************************************/
WindowsWidget::WindowsWidget(const ScreenConfiguration &screenConfiguration, AppWindow *parent)
    : QWidget(parent)
    , m_appWindow(parent)
    , m_screenConfiguration(screenConfiguration)
{
    ui.setupUi(this);
}

WindowsWidget::WindowInfo *WindowsWidget::findExistingWindowInfo(const WindowInfo &windowInfo)
{
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it)
        if (it->sameAs(windowInfo))
            return &*it;
    return nullptr;
}

int __stdcall WindowsWidget::enumWindowsCallback(HWND winId, LPARAM userParam)
{
    WindowInfoBase windowInfoBase;
    if (WindowInfoBase::fromWinId(winId, &windowInfoBase)) {
        WindowInfo windowInfo(windowInfoBase);
        WindowsWidget *windowsWidget = reinterpret_cast<WindowsWidget *>(userParam);
        windowsWidget->m_tempWindowsInfo << windowInfo;
    }

    return true;
}

void WindowsWidget::capture()
{
    QStringList msg = { "<i>Capturing...</i>" };

    m_tempWindowsInfo.clear();
    if (!::EnumWindows(enumWindowsCallback, reinterpret_cast<LPARAM>(this))) {
        m_appWindow->log("EnumWindows returned false!");
        return;
    }

    int previousEntryCount = m_windows.count();
    bool changedTable = false;

    QSet<QTableWidgetItem *> rowsToDelete;
    for (const WindowInfo &windowInfo : m_windows)
        rowsToDelete << windowInfo.tableWidgetItems.first();

    for (const WindowInfo &capturedWindowInfo : m_tempWindowsInfo) {
        if (WindowInfo *const existingWindowInfo = findExistingWindowInfo(capturedWindowInfo)) {
            switch (capturedWindowInfo.recommendedAction()) {
                case Update:
                    existingWindowInfo->update(capturedWindowInfo, ui.tableWidget, changedTable);
                    // fallthrough
                case Keep:
                    rowsToDelete.remove(existingWindowInfo->tableWidgetItems.first());
                    break;
                case Discard:
                    break;
            }
        } else {
            switch (capturedWindowInfo.recommendedAction()) {
                case Update:
                case Keep:
                    m_windows.prepend(capturedWindowInfo);
                    m_windows.begin()->update(capturedWindowInfo, ui.tableWidget, changedTable);
                    break;
                case Discard:
                    break;
            }
        }
    }

    for (QTableWidgetItem *rowToDelete : rowsToDelete) {
        WindowInfo windowInfo = rowToDelete->data(Qt::UserRole).value<WindowInfo>();
        m_appWindow->log(QSL("Forgetting window: %1 %2").arg(reinterpret_cast<quint64>(windowInfo.hwnd)).arg(windowInfo.executableFilePath));
        windowInfo.forget();
    }

    if (changedTable) {
        ui.tableWidget->resizeColumnsToContents();
        ui.tableWidget->resizeRowsToContents();
    }

    msg << QSL("<b>Current -&gt; raw -&gt; captured windows :</b> %1 -&gt; %2 -&gt; %3").arg(previousEntryCount).arg(m_tempWindowsInfo.count()).arg(m_windows.count());
    m_appWindow->log(msg);
}

void WindowsWidget::restore(const WindowInfo *const singleWindowInfo)
{
    for (const WindowInfo &windowInfo : m_windows) {
        if (singleWindowInfo && !singleWindowInfo->sameAs(windowInfo))
            continue;
        ::SetWindowPos(windowInfo.hwnd, HWND_TOP, windowInfo.geometry.x(), windowInfo.geometry.y(), windowInfo.geometry.width(), windowInfo.geometry.height(), SWP_NOZORDER);
        if (windowInfo.windowState == Qt::WindowMaximized)
            ::ShowWindow(windowInfo.hwnd, SW_MAXIMIZE);/*
        else if (windowInfo.windowState == Qt::WindowMinimized)
            ::ShowWindow(windowInfo.hwnd, SW_MINIMIZE);
        else
            ::ShowWindow(windowInfo.hwnd, SW_NORMAL);*/
    }
}

void WindowsWidget::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    item = item->tableWidget()->item(item->row(), 0);
    const WindowInfo windowInfo = item->data(Qt::UserRole).value<WindowInfo>();
    restore(&windowInfo);
    //::BringWindowToTop(windowInfo.hwnd);
}
