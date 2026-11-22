#include "include/gui/MainWindow.h"
#include "include/gui/RoleSelection.h"          
#include <cstring>
int main(int argc, char* argv[])
{
    
    sf::RenderWindow window(sf::VideoMode(1024, 768), "ISE POS");
    RoleSelection roleSel(window);

    // Keep showing the role screen until the user picks Cashier or Manager
    while (window.isOpen() && g_currentRole == UserRole::None) {
        if (roleSel.run()) break;          // returns true when a role is chosen
    }
    if (!window.isOpen()) return 0;        // user closed the window during role pick

    MainWindow app;
    app.run();
    return 0;
}
