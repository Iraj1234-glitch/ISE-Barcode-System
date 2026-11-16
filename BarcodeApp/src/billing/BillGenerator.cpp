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
    total = 0;
}

// Add product to bill
bool BillGenerator::addProduct(string code, string name,
    double price, int quantity)
{
    // Validate product information
    if (code == "" || name == "" || price < 0 || quantity <= 0)
    {
        cout << "Invalid product information!" << endl;
        return false;
    }

    double subtotal = price * quantity;

    // Create product record
    stringstream record;

    record << code << ","
        << name << ","
        << fixed << setprecision(2) << price << ","
        << quantity << ","
        << subtotal;

    // Separate products using |
    if (billRecord != "")
    {
        billRecord += "|";
    }

    billRecord += record.str();

    // Update total
    total += subtotal;

    return true;
}

// Display bill
void BillGenerator::displayBill()
{
    cout << "\n========== CUSTOMER BILL ==========\n";

    cout << billRecord << endl;

    cout << "-----------------------------------\n";

    cout << "Total Amount: Rs. "
        << fixed << setprecision(2) << total << endl;

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
    total = 0;
}