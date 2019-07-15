#include "appwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WinPosKeeper");
    app.setWindowIcon(QIcon(":/appicon.png"));

    AppWindow appWindow;
    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QCoreApplication::quit);

#ifdef QT_DEBUG
    appWindow.show();
#endif

    return app.exec();
}
