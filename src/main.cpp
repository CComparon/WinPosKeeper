#include "appwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WinPosKeeper");
    app.setWindowIcon(QIcon(":/appicon.png"));

    AppWindow appWindow;
    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QCoreApplication::quit);

    appWindow.show();    
    return app.exec();
}
