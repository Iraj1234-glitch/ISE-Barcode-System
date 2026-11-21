#include "include/gui/MainWindow.h"
#include "include/gui/ManagerProductWindow.h"
#include <cstring>

int main(int argc, char* argv[])
{
    if (argc > 1 && std::strcmp(argv[1], "--manager") == 0)
    {
        ManagerProductWindow managerApp;
        managerApp.run();
        return 0;
    }

    MainWindow app;
    app.run();
    return 0;
}
