#pragma once

#include <string>
using namespace std;

class BillGenerator
{
private:
    string billRecord;
    string fileName;
    double total;

public:
    BillGenerator();

    // Add a product to the bill
    bool addProduct(string code, string name, double price, int quantity);

    // Display bill
    void displayBill();

    // Save bill in file
    bool saveBill();

    // Start a new bill
    void resetBill();
};