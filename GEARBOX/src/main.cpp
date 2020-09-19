#include "GearBox.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QSplashScreen.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //SplashScreen
    QPixmap splashScreenImage("../Branding/GEAR_logo_dark.png");
    QSplashScreen splashScreen(splashScreenImage);
    splashScreen.show();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    splashScreen.hide();

    GearBox w;
    w.show();
    return app.exec();
}
