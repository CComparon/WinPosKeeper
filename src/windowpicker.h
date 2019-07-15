#ifndef WINDOW_PICKER_H
#define WINDOW_PICKER_H

/*************************************************************************************************************************/
class WindowPicker : public QPushButton
{
    Q_OBJECT

public:
    WindowPicker(QWidget *parent = nullptr);

private:
    void mousePressEvent(QMouseEvent *e) final;
    void mouseMoveEvent(QMouseEvent *e) final;
    void mouseReleaseEvent(QMouseEvent *e) final;

signals:
    void windowPicked(HWND winId);

private:
    const QPixmap m_pixmap = QPixmap(":/aim.png");
    const QCursor m_cursor;
    bool m_dragging = false;
};

#endif
