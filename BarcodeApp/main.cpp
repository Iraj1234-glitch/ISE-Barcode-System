
#include <iostream>
#include <string>
#include <cstring>

#include "include/generation/BarcodeGenerator.h"
#include "include/gui/MainWindow.h"
#include "include/gui/ManagerProductWindow.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc > 1 && strcmp(argv[1], "--manager") == 0)
    {
        ManagerProductWindow managerApp;
        managerApp.run();
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "--barcode-test") == 0)
    {
        BarcodeGenerator generator;

        string code;

        cout << "Enter a 6-digit number: ";
        cin >> code;

        if (generator.generateBarcode(code, "barcode.bmp"))
        {
            cout << "Barcode generated successfully!" << endl;
            cout << "Saved as barcode.bmp" << endl;
        }
        else
        {
            cout << "Invalid input or barcode generation failed!" << endl;
        }

        return 0;
    }

    MainWindow app;
    app.run();

    return 0;
}
