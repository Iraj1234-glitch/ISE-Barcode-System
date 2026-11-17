#include "../BarcodeApp/include/product/ProductManager.h"
#include "../BarcodeApp/include/order/Order.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>

#define TEST_ASSERT(cond, msg) \
    if (!(cond)) { \
        std::cerr << "FAIL: " << msg << std::endl; \
        return false; \
    } else { \
        std::cout << "PASS: " << msg << std::endl; \
    }

bool runAllProductTests() {
    std::cout << "=========================================" << std::endl;
    std::cout << "STARTING PRODUCT MANAGEMENT MODULE TESTS" << std::endl;
    std::cout << "=========================================" << std::endl;

    const std::string sampleProductFile = "Product.txt";
    const std::string testProductFile = "test_Product.txt";
    const std::string testInventoryFile = "test_Inventory.txt";
    const std::string emptyFile = "test_empty_Product.txt";
    const std::string malformedFile = "test_malformed_Product.txt";

    // Clean up test files if they already exist
    std::remove(testProductFile.c_str());
    std::remove(testInventoryFile.c_str());
    std::remove(emptyFile.c_str());
    std::remove(malformedFile.c_str());

    // --------------------------------------------------
    // Test Category A: Product Loading Tests (Sample Product.txt)
    // --------------------------------------------------
    std::cout << "\n--- Category A: Product Loading Tests ---" << std::endl;
    ProductManager manager(sampleProductFile);
    bool loaded = manager.loadProducts();
    TEST_ASSERT(loaded == true, "Product.txt loaded successfully");
    TEST_ASSERT(manager.getProductCount() == 6, "All 6 supplied sample products loaded");

    // Verify Sample 1: P001 - Barcode 100001 - Milk 1L - Rs. 280.00
    const Product* p1 = manager.findProductById("P001");
    TEST_ASSERT(p1 != nullptr, "P001 found in catalog");
    TEST_ASSERT(p1->getBarcode() == "100001", "P001 barcode is 100001");
    TEST_ASSERT(p1->getProductName() == "Milk 1L", "P001 name is 'Milk 1L'");
    TEST_ASSERT(p1->getDescription() == "Fresh full-cream milk in a 1 litre pack. Keep refrigerated.", "P001 description matches exactly");
    TEST_ASSERT(std::abs(p1->getPrice() - 280.00) < 0.001, "P001 price is 280.00");

    // Verify Sample 2: P002 - Barcode 100002 - Bread - Rs. 120.00
    const Product* p2 = manager.findProductById("P002");
    TEST_ASSERT(p2 != nullptr, "P002 found in catalog");
    TEST_ASSERT(p2->getBarcode() == "100002", "P002 barcode is 100002");
    TEST_ASSERT(p2->getProductName() == "Bread", "P002 name is 'Bread'");
    TEST_ASSERT(p2->getDescription() == "Sliced white bread, 400 g loaf. Best before 3 days from purchase.", "P002 description matches exactly");
    TEST_ASSERT(std::abs(p2->getPrice() - 120.00) < 0.001, "P002 price is 120.00");

    // Verify Sample 3: P003 - Barcode 100003 - Eggs (12) - Rs. 360.00
    const Product* p3 = manager.findProductById("P003");
    TEST_ASSERT(p3 != nullptr, "P003 found in catalog");
    TEST_ASSERT(p3->getBarcode() == "100003", "P003 barcode is 100003");
    TEST_ASSERT(p3->getProductName() == "Eggs (12)", "P003 name is 'Eggs (12)'");
    TEST_ASSERT(p3->getDescription() == "Farm eggs, one dozen in a protective tray.", "P003 description matches exactly");
    TEST_ASSERT(std::abs(p3->getPrice() - 360.00) < 0.001, "P003 price is 360.00");

    // Verify Sample 4: P004 - Barcode 100004 - Rice 5kg - Rs. 1450.00
    const Product* p4 = manager.findProductById("P004");
    TEST_ASSERT(p4 != nullptr, "P004 found in catalog");
    TEST_ASSERT(p4->getBarcode() == "100004", "P004 barcode is 100004");
    TEST_ASSERT(p4->getProductName() == "Rice 5kg", "P004 name is 'Rice 5kg'");
    TEST_ASSERT(p4->getDescription() == "Premium long-grain basmati rice in a 5 kg bag.", "P004 description matches exactly");
    TEST_ASSERT(std::abs(p4->getPrice() - 1450.00) < 0.001, "P004 price is 1450.00");

    // Verify Sample 5: P005 - Barcode 100005 - Cooking Oil 1L - Rs. 520.00
    const Product* p5 = manager.findProductById("P005");
    TEST_ASSERT(p5 != nullptr, "P005 found in catalog");
    TEST_ASSERT(p5->getBarcode() == "100005", "P005 barcode is 100005");
    TEST_ASSERT(p5->getProductName() == "Cooking Oil 1L", "P005 name is 'Cooking Oil 1L'");
    TEST_ASSERT(p5->getDescription() == "Refined sunflower cooking oil, 1 litre bottle.", "P005 description matches exactly");
    TEST_ASSERT(std::abs(p5->getPrice() - 520.00) < 0.001, "P005 price is 520.00");

    // Verify Sample 6: P006 - Barcode 100006 - Sugar 1kg - Rs. 170.00
    const Product* p6 = manager.findProductById("P006");
    TEST_ASSERT(p6 != nullptr, "P006 found in catalog");
    TEST_ASSERT(p6->getBarcode() == "100006", "P006 barcode is 100006");
    TEST_ASSERT(p6->getProductName() == "Sugar 1kg", "P006 name is 'Sugar 1kg'");
    TEST_ASSERT(p6->getDescription() == "Fine white sugar, 1 kg pack.", "P006 description matches exactly");
    TEST_ASSERT(std::abs(p6->getPrice() - 170.00) < 0.001, "P006 price is 170.00");

    // Missing file test
    ProductManager missingManager("non_existent_product_file.txt");
    bool missingLoaded = missingManager.loadProducts();
    TEST_ASSERT(missingLoaded == false, "Missing file safely returns false without crashing");
    TEST_ASSERT(missingManager.getProductCount() == 0, "Missing file results in 0 loaded products");

    // Empty file test
    std::ofstream ef(emptyFile);
    ef.close();
    ProductManager emptyManager(emptyFile);
    bool emptyLoaded = emptyManager.loadProducts();
    TEST_ASSERT(emptyLoaded == true, "Empty file handled safely returning true");
    TEST_ASSERT(emptyManager.getProductCount() == 0, "Empty file results in 0 products");

    // Malformed records test
    std::ofstream mf(malformedFile);
    mf << "P999 999999 \"OnlyThreeFields\" #\n"; // Missing description and price
    mf << "P888 888888 \"InvalidPrice\" \"Desc\" not_a_number #\n"; // Non-numeric price
    mf << "P777 777777 \"Valid Product\" \"Valid Description\" 99.50 #\n"; // Valid record
    mf.close();
    ProductManager malformedManager(malformedFile);
    bool mfLoaded = malformedManager.loadProducts();
    TEST_ASSERT(mfLoaded == true, "Malformed file processed without crashing");
    TEST_ASSERT(malformedManager.getProductCount() == 1, "Only the 1 valid record was loaded; malformed records were safely skipped");
    const Product* validProd = malformedManager.findProductById("P777");
    TEST_ASSERT(validProd != nullptr && validProd->getProductName() == "Valid Product", "Valid record was loaded with intact attributes");

    // --------------------------------------------------
    // Test Category B: Barcode Lookup Tests
    // --------------------------------------------------
    std::cout << "\n--- Category B: Barcode-Based Product Lookup Tests ---" << std::endl;
    // Search barcode 100001
    const Product* found1 = manager.findProductByBarcode("100001");
    TEST_ASSERT(found1 != nullptr && found1->getProductId() == "P001" && found1->getProductName() == "Milk 1L",
                "Search barcode 100001 returned Milk 1L");

    // Search barcode 100002
    const Product* found2 = manager.findProductByBarcode("100002");
    TEST_ASSERT(found2 != nullptr && found2->getProductId() == "P002" && found2->getProductName() == "Bread",
                "Search barcode 100002 returned Bread");

    // Search barcode 100006
    const Product* found6 = manager.findProductByBarcode("100006");
    TEST_ASSERT(found6 != nullptr && found6->getProductId() == "P006" && found6->getProductName() == "Sugar 1kg",
                "Search barcode 100006 returned Sugar 1kg");

    // Search nonexistent barcode
    const Product* notFoundBarcode = manager.findProductByBarcode("999999");
    TEST_ASSERT(notFoundBarcode == nullptr, "Search nonexistent barcode 999999 correctly returned nullptr");

    // Search invalid barcode
    const Product* invalidBarcode = manager.findProductByBarcode("");
    TEST_ASSERT(invalidBarcode == nullptr, "Search empty barcode correctly returned nullptr");

    // Barcode format validation helper tests
    TEST_ASSERT(ProductManager::isValidBarcodeFormat("100001") == true, "100001 is a valid Code 128 6-digit barcode");
    TEST_ASSERT(ProductManager::isValidBarcodeFormat("012345") == false, "012345 rejected (first digit zero)");
    TEST_ASSERT(ProductManager::isValidBarcodeFormat("12345") == false, "12345 rejected (less than 6 digits)");
    TEST_ASSERT(ProductManager::isValidBarcodeFormat("1234567") == false, "1234567 rejected (more than 6 digits)");
    TEST_ASSERT(ProductManager::isValidBarcodeFormat("123A56") == false, "123A56 rejected (non-numeric)");

    // --------------------------------------------------
    // Test Category C: Description Retrieval Tests
    // --------------------------------------------------
    std::cout << "\n--- Category C: Product Description Retrieval Tests ---" << std::endl;
    std::string descMilk = manager.getProductDescriptionByBarcode("100001");
    TEST_ASSERT(descMilk == "Fresh full-cream milk in a 1 litre pack. Keep refrigerated.",
                "Milk 1L description retrieved accurately via barcode");

    std::string descBread = manager.getProductDescriptionByBarcode("100002");
    TEST_ASSERT(descBread == "Sliced white bread, 400 g loaf. Best before 3 days from purchase.",
                "Bread description retrieved accurately via barcode");

    std::string descRice = manager.getProductDescriptionByBarcode("100004");
    TEST_ASSERT(descRice == "Premium long-grain basmati rice in a 5 kg bag.",
                "Rice 5kg description retrieved accurately via barcode");

    std::string descNonExistent = manager.getProductDescriptionByBarcode("999999");
    TEST_ASSERT(descNonExistent.empty(), "Nonexistent product description returns empty string safely");

    // Full product details string for Hiba GUI
    std::string fullDetails = manager.getFullProductDetails("100001");
    TEST_ASSERT(fullDetails.find("Milk 1L") != std::string::npos &&
                fullDetails.find("280.00") != std::string::npos &&
                fullDetails.find("P001") != std::string::npos,
                "getFullProductDetails contains all product attributes");

    // --------------------------------------------------
    // Test Category D: Stock Validation Tests (No deduction!)
    // --------------------------------------------------
    std::cout << "\n--- Category D: Stock Validation Tests ---" << std::endl;
    // Set documented test stock values
    manager.updateStockQuantity("100001", 20); // 20 units of Milk
    manager.updateStockQuantity("100002", 10); // 10 units of Bread

    // Case 1: Product exists and sufficient stock is available (qty < stock)
    StockValidationResult res1 = manager.validateStockByBarcode("100001", 5);
    TEST_ASSERT(res1.code == StockValidationCode::VALID && res1.isValid == true,
                "Case 1a: Requested quantity smaller than available stock returns VALID");
    TEST_ASSERT(manager.getStockByBarcode("100001") == 20,
                "Stock validation did NOT deduct stock (stock remains 20)");

    // Case 1b: Quantity exactly equal to available stock (qty == stock)
    StockValidationResult res1b = manager.validateStockByBarcode("100001", 20);
    TEST_ASSERT(res1b.code == StockValidationCode::VALID && res1b.isValid == true,
                "Case 1b: Requested quantity equal to available stock returns VALID");

    // Case 2: Product exists but stock is insufficient (qty > stock)
    StockValidationResult res2 = manager.validateStockByBarcode("100001", 25);
    TEST_ASSERT(res2.code == StockValidationCode::INSUFFICIENT_STOCK && res2.isValid == false,
                "Case 2: Requested quantity greater than available stock returns INSUFFICIENT_STOCK");
    TEST_ASSERT(res2.availableStock == 20 && res2.requestedQuantity == 25,
                "Validation result correctly reports available stock (20) and requested quantity (25)");

    // Case 3: Product does not exist
    StockValidationResult res3 = manager.validateStockByBarcode("999999", 1);
    TEST_ASSERT(res3.code == StockValidationCode::PRODUCT_NOT_FOUND && res3.isValid == false,
                "Case 3: Nonexistent product returns PRODUCT_NOT_FOUND");

    // Case 4: Requested quantity is zero
    StockValidationResult res4 = manager.validateStockByBarcode("100001", 0);
    TEST_ASSERT(res4.code == StockValidationCode::ZERO_QUANTITY && res4.isValid == false,
                "Case 4: Zero quantity returns ZERO_QUANTITY");

    // Case 5: Requested quantity is negative
    StockValidationResult res5 = manager.validateStockByBarcode("100001", -5);
    TEST_ASSERT(res5.code == StockValidationCode::NEGATIVE_QUANTITY && res5.isValid == false,
                "Case 5: Negative quantity returns NEGATIVE_QUANTITY");

    // Case 6: Invalid identifier
    StockValidationResult res6 = manager.validateStockByBarcode("", 2);
    TEST_ASSERT(res6.code == StockValidationCode::INVALID_IDENTIFIER && res6.isValid == false,
                "Case 6: Empty identifier returns INVALID_IDENTIFIER");

    // Verify stock is still 20 after all validation calls
    TEST_ASSERT(manager.getStockByBarcode("100001") == 20,
                "Inventory verified unchanged across all stock validation tests");

    // --------------------------------------------------
    // Test Category E: Inventory and Quantity Updates
    // --------------------------------------------------
    std::cout << "\n--- Category E: Inventory & Quantity Updates Tests ---" << std::endl;
    // Set test quantities for all products
    manager.updateStockQuantity("P001", 50);
    manager.updateStockQuantity("P002", 30);
    manager.updateStockQuantity("P003", 60);
    manager.updateStockQuantity("P004", 25);
    manager.updateStockQuantity("P005", 40);
    manager.updateStockQuantity("P006", 100);

    TEST_ASSERT(manager.getStockById("P001") == 50, "P001 stock set to 50");
    TEST_ASSERT(manager.getStockById("P002") == 30, "P002 stock set to 30");

    // Prevent negative inventory
    bool negUpdate = manager.updateStockQuantity("P001", -10);
    TEST_ASSERT(negUpdate == false, "updateStockQuantity rejects negative value");
    TEST_ASSERT(manager.getStockById("P001") == 50, "P001 stock remains 50 after rejected negative update");

    // Add and deduct stock
    bool added = manager.addStock("P001", 10);
    TEST_ASSERT(added == true, "Added 10 units to P001");
    TEST_ASSERT(manager.getStockById("P001") == 60, "P001 stock is now 60 (50 + 10)");

    bool deducted = manager.deductStock("P001", 15);
    TEST_ASSERT(deducted == true, "Deducted 15 units from P001");
    TEST_ASSERT(manager.getStockById("P001") == 45, "P001 stock is now 45 (60 - 15)");

    // Save updated inventory to test file
    bool saved = manager.saveProducts(testProductFile, true);
    TEST_ASSERT(saved == true, "Saved products with updated inventory to test_Product.txt");

    // Reload from file to verify persistence
    ProductManager reloadedManager(testProductFile);
    bool reloadedOk = reloadedManager.loadProducts();
    TEST_ASSERT(reloadedOk == true, "Reloaded test_Product.txt successfully");
    TEST_ASSERT(reloadedManager.getProductCount() == 6, "Reloaded catalog has 6 products");
    TEST_ASSERT(reloadedManager.getStockById("P001") == 45, "Reloaded P001 stock persisted as 45");
    TEST_ASSERT(reloadedManager.getStockById("P002") == 30, "Reloaded P002 stock persisted as 30");

    // Verify all other product attributes were preserved unchanged
    const Product* rp1 = reloadedManager.findProductById("P001");
    TEST_ASSERT(rp1 != nullptr, "Reloaded P001 found");
    TEST_ASSERT(rp1->getBarcode() == "100001", "Barcode preserved as 100001");
    TEST_ASSERT(rp1->getProductName() == "Milk 1L", "Product name preserved as 'Milk 1L'");
    TEST_ASSERT(rp1->getDescription() == "Fresh full-cream milk in a 1 litre pack. Keep refrigerated.", "Description preserved");
    TEST_ASSERT(std::abs(rp1->getPrice() - 280.00) < 0.001, "Price preserved as 280.00");

    // --------------------------------------------------
    // Test Category F: Iraj's Order Management Workflow Integration
    // --------------------------------------------------
    std::cout << "\n--- Category F: Order Management Workflow Integration Tests ---" << std::endl;
    // Create an Order object (from Order.h)
    Order testOrder("10000115", "2026-11-15", "10:30:00", "John Doe", "03001234567", "in-progress", 0.0, "cash", "None");
    testOrder.addProduct("P001", "100001", 5);  // 5 units of Milk (stock: 45)
    testOrder.addProduct("P002", "100002", 10); // 10 units of Bread (stock: 30)

    // Process order stock deduction
    bool orderDeducted = reloadedManager.processOrderDeduction(testOrder);
    TEST_ASSERT(orderDeducted == true, "processOrderDeduction successfully processed Order 10000115");
    TEST_ASSERT(reloadedManager.getStockById("P001") == 40, "P001 stock correctly deducted to 40 (45 - 5)");
    TEST_ASSERT(reloadedManager.getStockById("P002") == 20, "P002 stock correctly deducted to 20 (30 - 10)");
    TEST_ASSERT(reloadedManager.isOrderProcessed("10000115") == true, "Order 10000115 recorded as processed");

    // Guard test: Prevent double deduction
    bool doubleDeduct = reloadedManager.processOrderDeduction(testOrder);
    TEST_ASSERT(doubleDeduct == false, "Re-processing the same order was correctly rejected (no double deduction!)");
    TEST_ASSERT(reloadedManager.getStockById("P001") == 40, "P001 stock remained 40 (not deducted twice)");
    TEST_ASSERT(reloadedManager.getStockById("P002") == 20, "P002 stock remained 20 (not deducted twice)");

    // Atomic transaction test: Order requesting more than available stock
    Order excessiveOrder("10000215", "2026-11-15", "11:00:00", "Alice", "", "in-progress", 0.0, "cash", "None");
    excessiveOrder.addProduct("P001", "100001", 2);   // Available: 40 (valid)
    excessiveOrder.addProduct("P002", "100002", 500); // Available: 20 (exceeds stock!)

    bool excessiveProcessed = reloadedManager.processOrderDeduction(excessiveOrder);
    TEST_ASSERT(excessiveProcessed == false, "Order exceeding stock for any product is completely rejected");
    TEST_ASSERT(reloadedManager.getStockById("P001") == 40, "P001 stock unchanged at 40 (atomic rollback: P001 was NOT deducted)");
    TEST_ASSERT(reloadedManager.getStockById("P002") == 20, "P002 stock unchanged at 20");

    // Order cancellation / stock restoration test
    bool restored = reloadedManager.restoreOrderStock(testOrder);
    TEST_ASSERT(restored == true, "restoreOrderStock successfully restored stock for Order 10000115");
    TEST_ASSERT(reloadedManager.getStockById("P001") == 45, "P001 stock restored back to 45 (40 + 5)");
    TEST_ASSERT(reloadedManager.getStockById("P002") == 30, "P002 stock restored back to 30 (20 + 10)");
    TEST_ASSERT(reloadedManager.isOrderProcessed("10000115") == false, "Order 10000115 is no longer in processed list");

    // Attempting to restore already restored order
    bool restoreAgain = reloadedManager.restoreOrderStock(testOrder);
    TEST_ASSERT(restoreAgain == false, "Restoring already restored order rejected");

    // --------------------------------------------------
    // Test Category G: Separate Inventory File Support (Optional Inventory.txt)
    // --------------------------------------------------
    std::cout << "\n--- Category G: Optional Inventory.txt Tests ---" << std::endl;
    bool invSaved = reloadedManager.saveInventory(testInventoryFile);
    TEST_ASSERT(invSaved == true, "Saved inventory to test_Inventory.txt");

    // Modify in-memory stock, then reload from Inventory.txt
    reloadedManager.updateStockQuantity("P001", 0);
    TEST_ASSERT(reloadedManager.getStockById("P001") == 0, "P001 temporarily set to 0 in memory");
    bool invLoaded = reloadedManager.loadInventory(testInventoryFile);
    TEST_ASSERT(invLoaded == true, "Loaded inventory from test_Inventory.txt");
    TEST_ASSERT(reloadedManager.getStockById("P001") == 45, "P001 stock restored from inventory file as 45");

    // Clean up temporary test files
    std::remove(testProductFile.c_str());
    std::remove(testInventoryFile.c_str());
    std::remove(emptyFile.c_str());
    std::remove(malformedFile.c_str());

    std::cout << "\n=========================================" << std::endl;
    std::cout << "ALL PRODUCT MANAGEMENT TESTS PASSED (100% SUCCESS)!" << std::endl;
    std::cout << "=========================================" << std::endl;
    return true;
}

int main() {
    bool ok = runAllProductTests();
    return ok ? 0 : 1;
}
