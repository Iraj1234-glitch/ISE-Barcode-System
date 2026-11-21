#pragma once

#include <string>
using namespace std;

class BillGenerator
{
private:
    string billRecord;
    string fileName;
    double subtotal;
    double totalTax;
    double totalDiscount;
    double total;

public:
    BillGenerator();

    // Add a product to the bill
    bool addProduct(string code, string name, double price, int quantity);
    bool addProduct(string code, string name, double price, int quantity, double taxPerUnit, double discountPerUnit);

    double getSubtotal() const;
    double getTotalTax() const;
    double getTotalDiscount() const;
    double getTotal() const;

    // Display bill
    void displayBill();

    // Save bill in file
    bool saveBill();

    // Start a new bill
    void resetBill();
};
