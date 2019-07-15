#include "windowpicker.h"
#include "utils.h"

WindowPicker::WindowPicker(QWidget *parent)
    : QPushButton(parent)
    , m_cursor(m_pixmap.scaledToWidth(48, Qt::SmoothTransformation), 24, 24)
{
    setIcon(m_pixmap);
}

void WindowPicker::mousePressEvent(QMouseEvent *e)
{
    m_dragging = true;
    qApp->setOverrideCursor(m_cursor);
    e->accept();
}

void WindowPicker::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_dragging)
        return;
    QPoint globalPos = e->globalPos();
    HWND hoveredWinId = ::WindowFromPoint({ globalPos.x(), globalPos.y() });
    WindowInfoBase windowInfo;
    if (WindowInfoBase::fromWinId(hoveredWinId, &windowInfo)) {
        qDebug() << "hover" << globalPos << "-" << hoveredWinId << "-" << windowInfo.geometry << "-" << windowInfo.windowTitle;
        e->accept();
    }
}

void WindowPicker::mouseReleaseEvent(QMouseEvent *e)
{
    if (!m_dragging)
        return;
    QPoint globalPos = e->globalPos();
    HWND hoveredWinId = ::WindowFromPoint({ globalPos.x(), globalPos.y() });
    WindowInfoBase windowInfo;
    if (WindowInfoBase::fromWinId(hoveredWinId, &windowInfo)) {
        qDebug() << "drop" << hoveredWinId;
        emit windowPicked(hoveredWinId);
        e->accept();
    }
    m_dragging = false;
    qApp->restoreOverrideCursor();
}
