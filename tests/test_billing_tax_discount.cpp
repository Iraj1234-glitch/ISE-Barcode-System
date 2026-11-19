#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include "../BarcodeApp/include/product/Product.h"
#include "../BarcodeApp/include/product/ProductManager.h"
#include "../BarcodeApp/include/billing/BillGenerator.h"

using namespace std;

void testTaxAndDiscountCalculations()
{
    cout << "--- 1. Testing Tax and Discount Loading from Product.txt ---" << endl;
    ProductManager pm("BarcodeApp/Product.txt");
    bool loaded = pm.loadProducts();
    assert(loaded);

    // Cooking Oil 1L (100005): price 520, tax 20, discount 0
    const Product* oil = pm.findProductByBarcode("100005");
    assert(oil != nullptr);
    assert(oil->getPrice() == 520.00);
    assert(oil->getTax() == 20.00);
    assert(oil->getDiscount() == 0.00);
    cout << "PASS: Cooking Oil 1L loaded -> Price: " << oil->getPrice()
         << ", Tax: " << oil->getTax() << ", Discount: " << oil->getDiscount() << endl;

    // Sugar 1kg (100006): price 170, tax 10, discount 0
    const Product* sugar = pm.findProductByBarcode("100006");
    assert(sugar != nullptr);
    assert(sugar->getPrice() == 170.00);
    assert(sugar->getTax() == 10.00);
    assert(sugar->getDiscount() == 0.00);
    cout << "PASS: Sugar 1kg loaded -> Price: " << sugar->getPrice()
         << ", Tax: " << sugar->getTax() << ", Discount: " << sugar->getDiscount() << endl;

    cout << "\n--- 2. Testing Formula Verification ---" << endl;
    // Suppose customer buys:
    // 2 x Cooking Oil 1L (price 520, tax 20, discount 0)
    // 3 x Sugar 1kg (price 170, tax 10, discount 0)
    int qtyOil = 2;
    int qtySugar = 3;

    double subtotal = (oil->getPrice() * qtyOil) + (sugar->getPrice() * qtySugar);
    double totalTax = (oil->getTax() * qtyOil) + (sugar->getTax() * qtySugar);
    double totalDiscount = (oil->getDiscount() * qtyOil) + (sugar->getDiscount() * qtySugar);
    double totalPayable = subtotal + totalTax - totalDiscount;

    assert(subtotal == (520.00 * 2) + (170.00 * 3)); // 1040 + 510 = 1550.00
    assert(totalTax == (20.00 * 2) + (10.00 * 3));   // 40 + 30 = 70.00
    assert(totalDiscount == 0.00);
    assert(totalPayable == 1550.00 + 70.00 - 0.00);  // 1620.00

    cout << "  Subtotal     = Rs. " << subtotal << " (Expected: 1550.00)" << endl;
    cout << "  Total Tax    = Rs. " << totalTax << " (Expected: 70.00)" << endl;
    cout << "  Total Disc   = Rs. " << totalDiscount << " (Expected: 0.00)" << endl;
    cout << "  Total Payable= Rs. " << totalPayable << " (Expected: 1620.00)" << endl;
    cout << "PASS: Formulas verified." << endl;

    cout << "\n--- 3. Testing BillGenerator with Tax and Discount ---" << endl;
    BillGenerator bg;
    bg.addProduct(oil->getBarcode(), oil->getProductName(), oil->getPrice(), qtyOil, oil->getTax(), oil->getDiscount());
    bg.addProduct(sugar->getBarcode(), sugar->getProductName(), sugar->getPrice(), qtySugar, sugar->getTax(), sugar->getDiscount());

    assert(bg.getSubtotal() == 1550.00);
    assert(bg.getTotalTax() == 70.00);
    assert(bg.getTotalDiscount() == 0.00);
    assert(bg.getTotal() == 1620.00);

    // Test saving bill
    remove("Bill.txt");
    bool saved = bg.saveBill();
    assert(saved);

    // Verify Bill.txt contents
    ifstream file("Bill.txt");
    assert(file.is_open());
    string line;
    getline(file, line);
    file.close();
    cout << "  Bill.txt contents: " << line << endl;
    assert(line.find("SUBTOTAL,1550.00") != string::npos);
    assert(line.find("TAX,70.00") != string::npos);
    assert(line.find("TOTAL,1620.00") != string::npos);
    assert(line.back() == '#');

    remove("Bill.txt");
    cout << "PASS: Bill.txt saved and verified successfully." << endl;
}

int main()
{
    cout << "=================================================" << endl;
    cout << "TAX, DISCOUNT AND BILL WORKFLOW VERIFICATION" << endl;
    cout << "=================================================" << endl;
    testTaxAndDiscountCalculations();
    cout << "\nALL BILLING, TAX & DISCOUNT CHECKS PASSED!" << endl;
    return 0;
}
