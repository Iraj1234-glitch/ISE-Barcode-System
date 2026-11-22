#include "../BarcodeApp/include/order/Order.h"
#include "../BarcodeApp/include/order/OrderManager.h"
#include "../BarcodeApp/include/product/Product.h"
#include "../BarcodeApp/include/product/ProductManager.h"
#include "../BarcodeApp/include/product/ProductFileHandler.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cassert>
#include <string>
#include <cmath>

#define TEST_ASSERT(cond, msg) \
    if (!(cond)) { \
        std::cerr << "FAIL: " << msg << std::endl; \
        return false; \
    } else { \
        std::cout << "PASS: " << msg << std::endl; \
    }

// -------------------------------------------------------------
// Test 1: Creating and opening orders
// -------------------------------------------------------------
bool test1_CreateAndOpenOrder() {
    std::cout << "\n--- Test 1: Creating and Opening Orders ---" << std::endl;
    const std::string oFile = "tests/tmp_test1_order.txt";
    const std::string odFile = "tests/tmp_test1_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-23");
    bool created = om.createOrder(oId, "2026-11-23", "10:15:00", "Alice Smith", "03001234567",
                                  "in-progress", 0.0, "cash", "None");
    TEST_ASSERT(created, "Order created successfully");

    Order openedOrder;
    bool opened = om.openOrder(oId, openedOrder);
    TEST_ASSERT(opened, "Existing order opened successfully");
    TEST_ASSERT(openedOrder.getOrderId() == oId, "Order ID matches opened order");
    TEST_ASSERT(openedOrder.getCustomerName() == "Alice Smith", "Customer name matches");
    TEST_ASSERT(openedOrder.getOrderStatus() == "in-progress", "Order status is in-progress");
    TEST_ASSERT(openedOrder.getDetailCount() == 0, "Initial order has 0 details");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 2: Editing an active order
// -------------------------------------------------------------
bool test2_EditActiveOrder() {
    std::cout << "\n--- Test 2: Editing an Active Order ---" << std::endl;
    const std::string oFile = "tests/tmp_test2_order.txt";
    const std::string odFile = "tests/tmp_test2_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-23");
    om.createOrder(oId, "2026-11-23", "10:30:00", "Bob Jones", "", "in-progress", 0.0, "cash", "None");

    std::string reason;
    TEST_ASSERT(om.canEditOrder(oId, false, "2026-11-23", &reason), "Active order is editable without manager approval");

    bool added = om.addProductToOrder(oId, "P001", "100001", 3);
    TEST_ASSERT(added, "Added product P001 to active order");

    Order* o = om.findOrder(oId);
    TEST_ASSERT(o != nullptr && o->getDetailCount() == 1, "Order now contains 1 detail line");
    TEST_ASSERT(o->getDetail(0).getQuantity() == 3, "Quantity of P001 is 3");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 3: Editing a paid order with manager approval within 7 days
// -------------------------------------------------------------
bool test3_EditPaidOrderWithApprovalWithin7Days() {
    std::cout << "\n--- Test 3: Editing a Paid Order with Approval Within 7 Days ---" << std::endl;
    const std::string oFile = "tests/tmp_test3_order.txt";
    const std::string odFile = "tests/tmp_test3_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-20");
    om.createOrder(oId, "2026-11-20", "12:00:00", "Charlie Brown", "", "in-progress", 0.0, "cash", "None");
    om.addProductToOrder(oId, "P001", "100001", 2);

    // Mark as paid on 2026-11-20
    bool paid = om.markOrderAsPaid(oId, 560.0, "cash", "None", "2026-11-20");
    TEST_ASSERT(paid, "Order marked as paid on 2026-11-20");

    // Current date: 2026-11-23 (3 days elapsed, <= 7 days)
    std::string reason;
    bool canEdit = om.canEditOrder(oId, true, "2026-11-23", &reason);
    TEST_ASSERT(canEdit, "Paid order can be edited with manager approval within 3 days");

    // Perform an edit with manager approval
    bool updated = om.updateProductQuantityInOrder(oId, "P001", 5, true, "2026-11-23");
    TEST_ASSERT(updated, "Successfully updated product quantity in paid order with manager approval");

    Order* o = om.findOrder(oId);
    TEST_ASSERT(o != nullptr && o->getDetail(0).getQuantity() == 5, "Product quantity updated to 5");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 4: Rejecting edits without manager approval
// -------------------------------------------------------------
bool test4_RejectPaidOrderEditWithoutManagerApproval() {
    std::cout << "\n--- Test 4: Rejecting Edits Without Manager Approval ---" << std::endl;
    const std::string oFile = "tests/tmp_test4_order.txt";
    const std::string odFile = "tests/tmp_test4_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-21");
    om.createOrder(oId, "2026-11-21", "14:00:00", "David Miller", "", "in-progress", 0.0, "cash", "None");
    om.addProductToOrder(oId, "P001", "100001", 2);
    om.markOrderAsPaid(oId, 560.0, "cash", "None", "2026-11-21");

    // Current date: 2026-11-23 (2 days elapsed). managerApproval = false!
    std::string reason;
    bool canEdit = om.canEditOrder(oId, false, "2026-11-23", &reason);
    TEST_ASSERT(!canEdit, "Paid order edit correctly rejected when manager approval is false");
    TEST_ASSERT(reason.find("Manager approval is strictly required") != std::string::npos, "Reason explicitly mentions manager approval requirement");

    // Attempting modification without approval must fail
    bool added = om.addProductToOrder(oId, "P002", "100002", 1, false, "2026-11-23");
    TEST_ASSERT(!added, "Adding product to paid order without approval rejected");

    bool updated = om.updateProductQuantityInOrder(oId, "P001", 10, false, "2026-11-23");
    TEST_ASSERT(!updated, "Updating quantity in paid order without approval rejected");

    bool removed = om.removeProductFromOrder(oId, "P001", false, "2026-11-23");
    TEST_ASSERT(!removed, "Removing product from paid order without approval rejected");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 5: Rejecting edits after 7 days
// -------------------------------------------------------------
bool test5_RejectPaidOrderEditAfter7Days() {
    std::cout << "\n--- Test 5: Rejecting Edits After 7 Days ---" << std::endl;
    const std::string oFile = "tests/tmp_test5_order.txt";
    const std::string odFile = "tests/tmp_test5_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-10");
    om.createOrder(oId, "2026-11-10", "09:00:00", "Eva Green", "", "in-progress", 0.0, "cash", "None");
    om.addProductToOrder(oId, "P001", "100001", 1);
    om.markOrderAsPaid(oId, 280.0, "cash", "None", "2026-11-10");

    // Current date: 2026-11-23 (13 days elapsed > 7 days). Even WITH manager approval!
    std::string reason;
    bool canEdit = om.canEditOrder(oId, true, "2026-11-23", &reason);
    TEST_ASSERT(!canEdit, "Paid order edit rejected when elapsed days exceed 7 days (even with manager approval)");
    TEST_ASSERT(reason.find("after 7 days") != std::string::npos, "Reason explicitly states 7-day limit expiration");

    bool updateAttempt = om.updateProductQuantityInOrder(oId, "P001", 5, true, "2026-11-23");
    TEST_ASSERT(!updateAttempt, "Updating product quantity after 7 days rejected");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 6: Holding and resuming orders
// -------------------------------------------------------------
bool test6_HoldAndResumeOrder() {
    std::cout << "\n--- Test 6: Holding and Resuming Orders ---" << std::endl;
    const std::string oFile = "tests/tmp_test6_order.txt";
    const std::string odFile = "tests/tmp_test6_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-23");
    om.createOrder(oId, "2026-11-23", "11:00:00", "Frank White", "03112223334", "in-progress", 0.0, "cash", "None");
    om.addProductToOrder(oId, "P001", "100001", 4);
    om.addProductToOrder(oId, "P002", "100002", 2);

    TEST_ASSERT(om.getOnHoldOrderCount() == 0, "No orders on hold initially");

    // Put order on hold
    bool held = om.holdOrder(oId);
    TEST_ASSERT(held, "Order successfully put on hold");
    TEST_ASSERT(om.getOnHoldOrderCount() == 1, "Exactly 1 order is on hold");

    Order* heldOrder = om.findOrder(oId);
    TEST_ASSERT(heldOrder != nullptr && heldOrder->getOrderStatus() == "on-hold", "Order status is 'on-hold'");

    // Resume the on-hold order
    bool resumed = om.resumeOrder(oId);
    TEST_ASSERT(resumed, "Order successfully resumed");
    TEST_ASSERT(om.getOnHoldOrderCount() == 0, "On-hold order count back to 0");

    Order* resumedOrder = om.findOrder(oId);
    TEST_ASSERT(resumedOrder != nullptr && resumedOrder->getOrderStatus() == "in-progress", "Resumed order status is 'in-progress'");
    TEST_ASSERT(resumedOrder->getDetailCount() == 2, "Both products preserved without loss");
    TEST_ASSERT(resumedOrder->getDetail(0).getQuantity() == 4, "Product P001 quantity preserved as 4");
    TEST_ASSERT(resumedOrder->getDetail(1).getQuantity() == 2, "Product P002 quantity preserved as 2");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 7: Adding, updating, and removing products from orders
// -------------------------------------------------------------
bool test7_AddUpdateRemoveProducts() {
    std::cout << "\n--- Test 7: Adding, Updating, and Removing Products from Orders ---" << std::endl;
    const std::string oFile = "tests/tmp_test7_order.txt";
    const std::string odFile = "tests/tmp_test7_detail.txt";
    std::remove(oFile.c_str());
    std::remove(odFile.c_str());

    OrderManager om(oFile, odFile);
    std::string oId = om.generateOrderId("2026-11-23");
    om.createOrder(oId, "2026-11-23", "11:30:00", "Grace Hopper", "", "in-progress", 0.0, "cash", "None");

    // 1. Add products
    om.addProductToOrder(oId, "P001", "100001", 2);
    om.addProductToOrder(oId, "P002", "100002", 5);
    Order* o = om.findOrder(oId);
    TEST_ASSERT(o->getDetailCount() == 2, "Added 2 distinct products");

    // 2. Add duplicate product (scanned again -> automatic increment)
    om.addProductToOrder(oId, "P001", "100001", 3);
    TEST_ASSERT(o->getDetailCount() == 2, "Detail count remains 2 (no duplicate row)");
    TEST_ASSERT(o->getProductQuantity("P001") == 5, "P001 quantity incremented to 5 (2 + 3)");

    // 3. Update quantity directly
    om.updateProductQuantityInOrder(oId, "P002", 12);
    TEST_ASSERT(o->getProductQuantity("P002") == 12, "P002 quantity updated to 12");

    // 4. Remove product from order
    bool removed = om.removeProductFromOrder(oId, "P001");
    TEST_ASSERT(removed, "Successfully removed product P001 from order");
    TEST_ASSERT(!o->hasProduct("P001"), "Order no longer contains P001");
    TEST_ASSERT(o->getDetailCount() == 1, "Detail count reduced to 1");
    TEST_ASSERT(o->hasProduct("P002"), "P002 remains intact");
    TEST_ASSERT(o->getProductQuantity("P002") == 12, "P002 quantity still 12");

    std::remove(oFile.c_str());
    std::remove(odFile.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 8: Viewing inventory
// -------------------------------------------------------------
bool test8_ViewInventory() {
    std::cout << "\n--- Test 8: Viewing Inventory ---" << std::endl;
    ProductManager pm("Product.txt");
    bool loaded = pm.loadProducts();
    TEST_ASSERT(loaded, "Product catalog loaded successfully");
    TEST_ASSERT(pm.getProductCount() >= 6, "At least 6 products available in inventory");

    // Verify manager can view all attributes: ID, barcode, name, stock
    for (int i = 0; i < pm.getProductCount(); i++) {
        const Product* p = pm.getProductAt(i);
        TEST_ASSERT(p != nullptr, "Product pointer is valid");
        TEST_ASSERT(!p->getProductId().empty(), "Product has valid ID");
        TEST_ASSERT(!p->getBarcode().empty(), "Product has valid Barcode");
        TEST_ASSERT(!p->getProductName().empty(), "Product has valid Name");
        TEST_ASSERT(p->getStockQuantity() >= 0, "Product has non-negative stock quantity");
    }

    // Search inventory using product ID and barcode
    const Product* byId = pm.findProductById("P001");
    const Product* byBarcode = pm.findProductByBarcode("100001");
    TEST_ASSERT(byId != nullptr && byBarcode != nullptr, "Both ID and Barcode search succeeded");
    TEST_ASSERT(byId->getProductId() == byBarcode->getProductId(), "ID and Barcode locate identical product");

    return true;
}

// -------------------------------------------------------------
// Test 9: Updating stock quantities
// -------------------------------------------------------------
bool test9_UpdateStockQuantities() {
    std::cout << "\n--- Test 9: Updating Stock Quantities ---" << std::endl;
    ProductManager pm("Product.txt");
    pm.loadProducts();

    std::string msg;
    // Direct stock update
    bool updated = pm.updateStockQuantity("P001", 100, msg);
    TEST_ASSERT(updated, "Manager set stock quantity to 100");
    TEST_ASSERT(pm.getStockById("P001") == 100, "Stock for P001 is now 100");
    TEST_ASSERT(msg.find("Success") != std::string::npos, "Clear success message received");

    // Add stock when new shipment arrives
    bool added = pm.addStock("P001", 50, msg);
    TEST_ASSERT(added, "Manager added 50 units of new stock");
    TEST_ASSERT(pm.getStockById("P001") == 150, "Stock for P001 is now 150 (100 + 50)");

    // Deduct stock
    bool deducted = pm.deductStock("P001", 20, msg);
    TEST_ASSERT(deducted, "Deducted 20 units from P001");
    TEST_ASSERT(pm.getStockById("P001") == 130, "Stock for P001 is now 130 (150 - 20)");

    return true;
}

// -------------------------------------------------------------
// Test 10: Rejecting negative quantities
// -------------------------------------------------------------
bool test10_RejectNegativeQuantities() {
    std::cout << "\n--- Test 10: Rejecting Negative Quantities ---" << std::endl;
    ProductManager pm("Product.txt");
    pm.loadProducts();
    pm.updateStockQuantity("P001", 50);

    std::string msg;
    bool negUpdate = pm.updateStockQuantity("P001", -15, msg);
    TEST_ASSERT(!negUpdate, "Updating stock to negative value correctly rejected");
    TEST_ASSERT(pm.getStockById("P001") == 50, "Stock remained 50, not modified");
    TEST_ASSERT(msg.find("cannot be negative") != std::string::npos, "Received clear error message about negative quantity");

    bool negAdd = pm.addStock("P001", -5, msg);
    TEST_ASSERT(!negAdd, "Adding negative stock correctly rejected");

    bool negDeduct = pm.deductStock("P001", -10, msg);
    TEST_ASSERT(!negDeduct, "Deducting negative quantity correctly rejected");

    StockValidationResult res = pm.validateStock("P001", -3);
    TEST_ASSERT(!res.isValid && res.code == StockValidationCode::NEGATIVE_QUANTITY, "Stock validation rejects negative quantity");

    // Non-existent product clear error message
    bool notFound = pm.updateStockQuantity("NON_EXISTENT_ID", 10, msg);
    TEST_ASSERT(!notFound, "Updating non-existent product rejected");
    TEST_ASSERT(msg.find("not found") != std::string::npos, "Received clear error message for missing product");

    return true;
}

// -------------------------------------------------------------
// Test 11: Rejecting orders with insufficient stock
// -------------------------------------------------------------
bool test11_RejectOrdersWithInsufficientStock() {
    std::cout << "\n--- Test 11: Rejecting Orders with Insufficient Stock ---" << std::endl;
    ProductManager pm("Product.txt");
    pm.loadProducts();
    pm.updateStockQuantity("P002", 10); // Bread has 10 units

    // Pre-check validation
    StockValidationResult res = pm.validateStockById("P002", 25);
    TEST_ASSERT(!res.isValid && res.code == StockValidationCode::INSUFFICIENT_STOCK, "validateStock correctly reports INSUFFICIENT_STOCK");
    TEST_ASSERT(res.availableStock == 10 && res.requestedQuantity == 25, "Result contains available and requested amounts");

    // Order with excessive quantity
    Order bigOrder("10000123", "2026-11-23", "12:00:00", "Harry", "", "in-progress", 0.0, "cash", "None");
    bigOrder.addProduct("P002", "100002", 25);

    bool processed = pm.processOrderDeduction(bigOrder);
    TEST_ASSERT(!processed, "Order requesting 25 units when only 10 available was rejected");
    TEST_ASSERT(pm.getStockById("P002") == 10, "Stock of P002 was not touched and remains 10");

    return true;
}

// -------------------------------------------------------------
// Test 12: Deducting stock correctly
// -------------------------------------------------------------
bool test12_DeductStockCorrectly() {
    std::cout << "\n--- Test 12: Deducting Stock Correctly ---" << std::endl;
    const std::string tmpProd = "tests/tmp_test12_prod.txt";
    std::remove(tmpProd.c_str());

    ProductManager pm("Product.txt");
    pm.loadProducts();
    pm.updateStockQuantity("P001", 50);
    pm.updateStockQuantity("P002", 30);
    pm.saveProducts(tmpProd);

    ProductManager testPm(tmpProd);
    testPm.loadProducts();

    Order order("10000123", "2026-11-23", "12:30:00", "Ivy", "", "in-progress", 0.0, "cash", "None");
    order.addProduct("P001", "100001", 10);
    order.addProduct("P002", "100002", 5);

    bool processed = testPm.processOrderDeduction(order);
    TEST_ASSERT(processed, "Order stock deduction processed successfully");
    TEST_ASSERT(testPm.getStockById("P001") == 40, "P001 deducted from 50 to 40");
    TEST_ASSERT(testPm.getStockById("P002") == 25, "P002 deducted from 30 to 25");

    std::remove(tmpProd.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 13: Preventing duplicate stock deductions
// -------------------------------------------------------------
bool test13_PreventDuplicateStockDeduction() {
    std::cout << "\n--- Test 13: Preventing Duplicate Stock Deductions ---" << std::endl;
    const std::string tmpProd = "tests/tmp_test13_prod.txt";
    std::remove(tmpProd.c_str());

    ProductManager pm("Product.txt");
    pm.loadProducts();
    pm.updateStockQuantity("P001", 40);
    pm.saveProducts(tmpProd);

    ProductManager testPm(tmpProd);
    testPm.loadProducts();

    Order order("10000123", "2026-11-23", "13:00:00", "Jack", "", "in-progress", 0.0, "cash", "None");
    order.addProduct("P001", "100001", 5);

    bool firstDeduction = testPm.processOrderDeduction(order);
    TEST_ASSERT(firstDeduction, "First deduction succeeded");
    TEST_ASSERT(testPm.getStockById("P001") == 35, "Stock is now 35");
    TEST_ASSERT(testPm.isOrderProcessed("10000123"), "Order recorded as processed");

    // Second deduction attempt with exact same order
    bool secondDeduction = testPm.processOrderDeduction(order);
    TEST_ASSERT(!secondDeduction, "Second deduction for same order ID correctly rejected");
    TEST_ASSERT(testPm.getStockById("P001") == 35, "Stock remained 35 (not deducted a second time!)");

    std::remove(tmpProd.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 14: Restoring stock correctly
// -------------------------------------------------------------
bool test14_RestoreStockCorrectly() {
    std::cout << "\n--- Test 14: Restoring Stock Correctly ---" << std::endl;
    const std::string tmpProd = "tests/tmp_test14_prod.txt";
    std::remove(tmpProd.c_str());

    ProductManager pm("Product.txt");
    pm.loadProducts();
    pm.updateStockQuantity("P001", 50);
    pm.saveProducts(tmpProd);

    ProductManager testPm(tmpProd);
    testPm.loadProducts();

    Order order("10000123", "2026-11-23", "13:30:00", "Karen", "", "in-progress", 0.0, "cash", "None");
    order.addProduct("P001", "100001", 15);

    testPm.processOrderDeduction(order);
    TEST_ASSERT(testPm.getStockById("P001") == 35, "Stock deducted to 35");

    // Now reverse / cancel the order and restore stock
    bool restored = testPm.restoreOrderStock(order);
    TEST_ASSERT(restored, "Stock restored successfully");
    TEST_ASSERT(testPm.getStockById("P001") == 50, "Stock for P001 restored back to original 50");
    TEST_ASSERT(!testPm.isOrderProcessed("10000123"), "Order is no longer in processed list");

    // Attempting to restore again should be rejected
    bool restoreAgain = testPm.restoreOrderStock(order);
    TEST_ASSERT(!restoreAgain, "Restoring already-restored order rejected");

    std::remove(tmpProd.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 15: Saving and loading inventory after restarting
// -------------------------------------------------------------
bool test15_SaveAndLoadInventoryAfterRestart() {
    std::cout << "\n--- Test 15: Saving and Loading Inventory After Restart ---" << std::endl;
    const std::string tmpProd = "tests/tmp_test15_prod.txt";
    std::remove(tmpProd.c_str());

    // Session 1: Run app, update stock and save
    {
        ProductManager pm1("Product.txt");
        pm1.loadProducts();
        pm1.updateStockQuantity("P001", 88);
        pm1.updateStockQuantity("P002", 44);
        bool saved = pm1.saveProducts(tmpProd);
        TEST_ASSERT(saved, "Inventory saved to persistent file in Session 1");
    }

    // Session 2: Application restart with new manager instance
    {
        ProductManager pm2(tmpProd);
        bool reloaded = pm2.loadProducts();
        TEST_ASSERT(reloaded, "Application restarted and loaded inventory from persistent file");
        TEST_ASSERT(pm2.getStockById("P001") == 88, "P001 stock persisted across restart as 88");
        TEST_ASSERT(pm2.getStockById("P002") == 44, "P002 stock persisted across restart as 44");

        const Product* p1 = pm2.findProductById("P001");
        TEST_ASSERT(p1->getProductName() == "Milk 1L", "Product name intact after restart");
        TEST_ASSERT(p1->getBarcode() == "100001", "Barcode intact after restart");
        TEST_ASSERT(p1->getPrice() == 280.0, "Price intact after restart");
    }

    std::remove(tmpProd.c_str());
    return true;
}

// -------------------------------------------------------------
// Test 16: Handling file-saving failures without corrupting existing data
// -------------------------------------------------------------
bool test16_HandleFileSaveFailureWithoutCorruption() {
    std::cout << "\n--- Test 16: Handling File-Saving Failures Without Corruption ---" << std::endl;
    ProductManager pm("Product.txt");
    pm.loadProducts();
    pm.updateStockQuantity("P001", 60);

    // Save initial state to a valid file
    const std::string validFile = "tests/tmp_test16_valid.txt";
    std::remove(validFile.c_str());
    pm.saveProducts(validFile);

    ProductManager fragilePm(validFile);
    fragilePm.loadProducts();
    TEST_ASSERT(fragilePm.getStockById("P001") == 60, "Initial stock is 60");

    // Intentionally point the file path to an illegal / non-writable directory path
    const std::string badPath = "Z:/NonExistentDrive/bad_dir/Product.txt";
    ProductManager failingPm(badPath);
    failingPm.addProduct(*fragilePm.findProductById("P001"));
    TEST_ASSERT(failingPm.getStockById("P001") == 60, "Failing PM has product with stock 60");

    Order order("10000123", "2026-11-23", "14:00:00", "Leo", "", "in-progress", 0.0, "cash", "None");
    order.addProduct("P001", "100001", 20);

    // Deduction should fail because saving fails, and in-memory stock must NOT be corrupted!
    bool success = failingPm.processOrderDeduction(order);
    TEST_ASSERT(!success, "processOrderDeduction safely returns false when file saving fails");
    TEST_ASSERT(failingPm.getStockById("P001") == 60, "Stock was atomically rolled back to 60 (no memory corruption!)");
    TEST_ASSERT(!failingPm.isOrderProcessed("10000123"), "Order was NOT marked as processed");

    std::remove(validFile.c_str());
    return true;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "RUNNING COMPLETE ORDER & INVENTORY TEST SUITE (16 TESTS)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    bool allPassed = true;
    allPassed = test1_CreateAndOpenOrder() && allPassed;
    allPassed = test2_EditActiveOrder() && allPassed;
    allPassed = test3_EditPaidOrderWithApprovalWithin7Days() && allPassed;
    allPassed = test4_RejectPaidOrderEditWithoutManagerApproval() && allPassed;
    allPassed = test5_RejectPaidOrderEditAfter7Days() && allPassed;
    allPassed = test6_HoldAndResumeOrder() && allPassed;
    allPassed = test7_AddUpdateRemoveProducts() && allPassed;
    allPassed = test8_ViewInventory() && allPassed;
    allPassed = test9_UpdateStockQuantities() && allPassed;
    allPassed = test10_RejectNegativeQuantities() && allPassed;
    allPassed = test11_RejectOrdersWithInsufficientStock() && allPassed;
    allPassed = test12_DeductStockCorrectly() && allPassed;
    allPassed = test13_PreventDuplicateStockDeduction() && allPassed;
    allPassed = test14_RestoreStockCorrectly() && allPassed;
    allPassed = test15_SaveAndLoadInventoryAfterRestart() && allPassed;
    allPassed = test16_HandleFileSaveFailureWithoutCorruption() && allPassed;

    std::cout << "\n==========================================================" << std::endl;
    if (allPassed) {
        std::cout << "SUCCESS: ALL 16 REQUIREMENTS TESTS PASSED (100%)!" << std::endl;
    } else {
        std::cerr << "FAILURE: SOME TESTS FAILED!" << std::endl;
    }
    std::cout << "==========================================================" << std::endl;

    return allPassed ? 0 : 1;
}
