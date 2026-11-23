
#include "include/gui/mainWindow.h"
#include "include/gui/RoleSelection.h"

int main(int argc, char* argv[])
{
    sf::RenderWindow window(sf::VideoMode(1024, 768), "ISE POS");

    RoleSelection roleSel(window);

    while (window.isOpen() && g_currentRole == UserRole::None) {
        if (roleSel.run()) break;
    }

    if (!window.isOpen()) return 0;

    MainWindow app;
    app.run();

    return 0;
}
