
#include <iostream>
#include "include/generation/BarcodeGenerator.h"

using namespace std;

int main()
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
