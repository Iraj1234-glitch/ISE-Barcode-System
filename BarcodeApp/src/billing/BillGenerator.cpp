#include "../../include/billing/BillGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// Constructor
BillGenerator::BillGenerator()
{
    billRecord = "";
    fileName = "Bill.txt";
    subtotal = 0.0;
    totalTax = 0.0;
    totalDiscount = 0.0;
    total = 0.0;
}

// Add product to bill (standard 4-argument version)
bool BillGenerator::addProduct(string code, string name,
    double price, int quantity)
{
    return addProduct(code, name, price, quantity, 0.0, 0.0);
}

// Add product to bill with tax and discount
bool BillGenerator::addProduct(string code, string name,
    double price, int quantity, double taxPerUnit, double discountPerUnit)
{
    // Validate product information
    if (code == "" || name == "" || price < 0 || quantity <= 0)
    {
        cout << "Invalid product information!" << endl;
        return false;
    }

    double itemSubtotal = price * quantity;
    double itemTax = (taxPerUnit >= 0 ? taxPerUnit : 0.0) * quantity;
    double itemDiscount = (discountPerUnit >= 0 ? discountPerUnit : 0.0) * quantity;
    double itemPayable = itemSubtotal + itemTax - itemDiscount;

    // Create product record
    stringstream record;

    record << code << ","
        << name << ","
        << fixed << setprecision(2) << price << ","
        << quantity << ","
        << itemSubtotal << ","
        << itemTax << ","
        << itemDiscount << ","
        << itemPayable;

    // Separate products using |
    if (billRecord != "")
    {
        billRecord += "|";
    }

    billRecord += record.str();

    // Update totals
    subtotal += itemSubtotal;
    totalTax += itemTax;
    totalDiscount += itemDiscount;
    total = subtotal + totalTax - totalDiscount;

    return true;
}

double BillGenerator::getSubtotal() const
{
    return subtotal;
}

double BillGenerator::getTotalTax() const
{
    return totalTax;
}

double BillGenerator::getTotalDiscount() const
{
    return totalDiscount;
}

double BillGenerator::getTotal() const
{
    return total;
}

// Display bill
void BillGenerator::displayBill()
{
    cout << "\n========== CUSTOMER BILL ==========\n";
    cout << billRecord << endl;
    cout << "-----------------------------------\n";
    cout << "Subtotal    : Rs. " << fixed << setprecision(2) << subtotal << endl;
    cout << "Total Tax   : Rs. " << fixed << setprecision(2) << totalTax << endl;
    cout << "Total Disc. : Rs. " << fixed << setprecision(2) << totalDiscount << endl;
    cout << "Total Amount: Rs. " << fixed << setprecision(2) << total << endl;
    cout << "===================================\n";
}

// Save bill in file
bool BillGenerator::saveBill()
{
    if (billRecord == "")
    {
        cout << "No products available in bill!" << endl;
        return false;
    }

    ofstream file(fileName.c_str(), ios::app);

    if (!file)
    {
        cout << "Error opening Bill.txt!" << endl;
        return false;
    }

    // Store complete bill in ONE LINE
    file << billRecord
        << "|SUBTOTAL," << fixed << setprecision(2) << subtotal
        << "|TAX," << fixed << setprecision(2) << totalTax
        << "|DISCOUNT," << fixed << setprecision(2) << totalDiscount
        << "|TOTAL," << fixed << setprecision(2) << total
        << "#" << endl;

    file.close();

    cout << "Bill saved successfully!" << endl;

    return true;
}

// Reset bill for next customer
void BillGenerator::resetBill()
{
    billRecord = "";
    subtotal = 0.0;
    totalTax = 0.0;
    totalDiscount = 0.0;
    total = 0.0;
}
