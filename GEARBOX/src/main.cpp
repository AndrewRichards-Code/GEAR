#include "GearBox.h"
#include <QtWidgets/QApplication.h>
#include <QtWidgets/QSplashScreen.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //SplashScreen
    QPixmap splashScreenImage("../Branding/GEAR_logo_dark.png");
    QSplashScreen splashScreen(splashScreenImage);
    splashScreen.show();

    GearBox w;
    w.show();
    
    splashScreen.hide();
    
    return app.exec();
}
