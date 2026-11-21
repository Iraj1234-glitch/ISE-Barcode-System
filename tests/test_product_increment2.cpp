#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include "../BarcodeApp/include/product/Product.h"
#include "../BarcodeApp/include/product/ProductFileHandler.h"
#include "../BarcodeApp/include/product/ProductManager.h"
#include "../BarcodeApp/include/order/Order.h"
#include "../BarcodeApp/include/order/OrderManager.h"

using namespace std;

void testLoadSixProducts() {
    cout << "[TEST 1] Loading 6 products from Product.txt..." << endl;
    ProductManager pm("BarcodeApp/Product.txt");
    bool loaded = pm.loadProducts();
    assert(loaded);
    assert(pm.getProductCount() == 6);

    const string expectedBarcodes[6] = { "100001", "100002", "100003", "100004", "100005", "100006" };
    const string expectedNames[6] = { "Milk 1L", "Bread", "Eggs (12)", "Rice 5kg", "Cooking Oil 1L", "Sugar 1kg" };
    const double expectedPrices[6] = { 280.00, 120.00, 360.00, 1450.00, 520.00, 170.00 };

    for (int i = 0; i < 6; i++) {
        const Product* p = pm.findProductByBarcode(expectedBarcodes[i]);
        assert(p != nullptr);
        assert(p->getProductName() == expectedNames[i]);
        assert(p->getPrice() == expectedPrices[i]);
        assert(!p->getDescription().empty());
        cout << "  - Product " << p->getProductId() << " (" << p->getBarcode() << "): "
             << p->getProductName() << " @ Rs " << p->getPrice()
             << " | Desc: " << p->getDescription().substr(0, 30) << "..." << endl;
    }
    cout << "PASS: All 6 products loaded and verified." << endl;
}

void testBarcodeLookup() {
    cout << "\n[TEST 2] Verifying Barcode Lookup & Description Retrieval..." << endl;
    ProductManager pm("BarcodeApp/Product.txt");
    pm.loadProducts();

    assert(pm.findProductByBarcode("100001")->getProductName() == "Milk 1L");
    assert(pm.findProductByBarcode("100002")->getProductName() == "Bread");
    assert(pm.findProductByBarcode("100003")->getProductName() == "Eggs (12)");
    assert(pm.findProductByBarcode("100004")->getProductName() == "Rice 5kg");
    assert(pm.findProductByBarcode("100005")->getProductName() == "Cooking Oil 1L");
    assert(pm.findProductByBarcode("100006")->getProductName() == "Sugar 1kg");

    // Invalid barcode lookup
    assert(pm.findProductByBarcode("999999") == nullptr);
    assert(pm.findProductByBarcode("") == nullptr);

    // Description retrieval
    string milkDesc = pm.getProductDescriptionByBarcode("100001");
    assert(milkDesc.find("Fresh full-cream milk") != string::npos);
    cout << "PASS: Barcode lookup and description retrieval verified." << endl;
}

void testTenFieldSerializationRoundTrip() {
    cout << "\n[TEST 3] Testing Increment 2 10-Field Serialization and Deserialization..." << endl;
    string testFile = "tests/test_10field_products.txt";
    ProductFileHandler handler;

    Product originalProds[2];
    originalProds[0] = Product("P001", "100001", "MLK1L", "Milk 1L", "weight", 280.00, 0.00, 0.00, 50, "barcodes/100001.bmp", "Fresh full-cream milk in a 1 litre pack. Keep refrigerated.");
    originalProds[1] = Product("P002", "100002", "BRD", "Bread", "count", 120.00, 0.00, 0.00, 40, "barcodes/100002.bmp", "Sliced white bread, 400 g loaf. Best before 3 days from purchase.");

    bool saved = handler.saveProducts(testFile, originalProds, 2, true);
    assert(saved);

    // Read back file content to check line ending with #
    ifstream checkFile(testFile);
    string line1, line2;
    getline(checkFile, line1);
    getline(checkFile, line2);
    checkFile.close();

    cout << "  Serialized line 1: " << line1 << endl;
    cout << "  Serialized line 2: " << line2 << endl;
    assert(line1.back() == '#');
    assert(line2.back() == '#');

    // Parse back
    Product loadedProds[10];
    int loadedCount = 0;
    bool loaded = handler.loadProducts(testFile, loadedProds, loadedCount, 10);
    assert(loaded);
    assert(loadedCount == 2);

    assert(loadedProds[0].getProductId() == "P001");
    assert(loadedProds[0].getBarcode() == "100001");
    assert(loadedProds[0].getAcronym() == "MLK1L");
    assert(loadedProds[0].getProductName() == "Milk 1L");
    assert(loadedProds[0].getMeasurementUnit() == "weight");
    assert(loadedProds[0].getPrice() == 280.00);
    assert(loadedProds[0].getTax() == 0.00);
    assert(loadedProds[0].getDiscount() == 0.00);
    assert(loadedProds[0].getStockQuantity() == 50);
    assert(loadedProds[0].getBarcodeImagePath() == "barcodes/100001.bmp");
    assert(!loadedProds[0].getDescription().empty());

    remove(testFile.c_str());
    cout << "PASS: 10-field serialization round-trip verified." << endl;
}

void testOrderStockDeductionIntegration() {
    cout << "\n[TEST 4] Testing Stock Deduction and Restoration with Order..." << endl;
    ProductManager pm("BarcodeApp/Product.txt");
    pm.loadProducts();

    // Set stock for testing
    pm.findProductByBarcode("100001")->setStockQuantity(20);
    assert(pm.getStockByBarcode("100001") == 20);

    Order order("ORD12345", "19/11/2026", "09:00:00", "Iraj", "03001234567",
                "in-progress", 560.00, "cash", "CashCounter");
    order.addProduct("P001", "100001", 2);

    bool deducted = pm.processOrderDeduction(order);
    assert(deducted);
    assert(pm.getStockByBarcode("100001") == 18);

    bool restored = pm.restoreOrderStock(order);
    assert(restored);
    assert(pm.getStockByBarcode("100001") == 20);

    cout << "PASS: Stock deduction and restoration verified." << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "RUNNING PRODUCT INCREMENT 2 VERIFICATION" << endl;
    cout << "========================================" << endl;
    testLoadSixProducts();
    testBarcodeLookup();
    testTenFieldSerializationRoundTrip();
    testOrderStockDeductionIntegration();
    cout << "\nALL TESTS PASSED SUCCESSFULLY!" << endl;
    return 0;
}
