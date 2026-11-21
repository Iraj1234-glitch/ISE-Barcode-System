#include "../BarcodeApp/include/order/OrderManager.h"
#include <iostream>
#include <fstream>
#include <cstdio>

#define TEST_ASSERT(cond, msg) \
    if (!(cond)) { \
        std::cerr << "FAIL: " << msg << std::endl; \
        return false; \
    } else { \
        std::cout << "PASS: " << msg << std::endl; \
    }

bool runAllTests() {
    std::cout << "=========================================" << std::endl;
    std::cout << "STARTING ORDER MANAGEMENT MODULE TESTS" << std::endl;
    std::cout << "=========================================" << std::endl;

    const std::string testOrderFile = "test_Order.txt";
    const std::string testDetailFile = "test_OrderDetail.txt";

    // Clean up test files if they already exist
    std::remove(testOrderFile.c_str());
    std::remove(testDetailFile.c_str());

    OrderManager* manager = new OrderManager(testOrderFile, testDetailFile);

    // --------------------------------------------------
    // Test 1: Order ID Generation and Validation
    // --------------------------------------------------
    std::cout << "\n--- Test 1: Order ID Generation and Validation ---" << std::endl;
    std::string date1 = "2026-11-15";
    std::string id1 = manager->generateOrderId(date1);
    TEST_ASSERT(id1 == "10000115", "Generated first Order ID ends with date day 15 (10000115)");
    TEST_ASSERT(id1.length() == 8, "Generated Order ID has exactly 8 digits");

    std::string date2 = "15-11-2026";
    std::string id2 = manager->generateOrderId(date2);
    TEST_ASSERT(id2 == "10000115", "Generated Order ID works with DD-MM-YYYY format");

    // --------------------------------------------------
    // Test 2: Order Creation (Space-Separated & Quoted Strings)
    // --------------------------------------------------
    std::cout << "\n--- Test 2: Order Creation ---" << std::endl;
    bool created1 = manager->createOrder(id1, "2026-11-15", "10:30:00", "John Doe",
                                         "03001234567", "in-progress", 0.0, "cash", "None");
    TEST_ASSERT(created1 == true, "Order 1 created successfully with customer name containing space");
    TEST_ASSERT(manager->getOrderCount() == 1, "Order count is 1");

    // Reject duplicate Order ID
    bool dupCreated = manager->createOrder(id1, "2026-11-15", "10:35:00", "Another Person",
                                           "", "in-progress", 0.0, "cash", "None");
    TEST_ASSERT(dupCreated == false, "Duplicate Order ID correctly rejected");

    // Reject invalid status
    bool invalidStatus = manager->createOrder("10000215", "2026-11-15", "10:40:00", "Bob",
                                              "", "invalid-status", 0.0, "cash", "None");
    TEST_ASSERT(invalidStatus == false, "Invalid order status correctly rejected");

    // Reject invalid payment mode
    bool invalidMode = manager->createOrder("10000215", "2026-11-15", "10:40:00", "Bob",
                                            "", "in-progress", 0.0, "crypto", "None");
    TEST_ASSERT(invalidMode == false, "Invalid payment mode correctly rejected");

    // Reject mismatched last 2 digits
    bool mismatchedId = manager->createOrder("10000299", "2026-11-15", "10:40:00", "Bob",
                                             "", "in-progress", 0.0, "cash", "None");
    TEST_ASSERT(mismatchedId == false, "Order ID with non-matching date digits rejected");

    // Create second order with empty optional contact and multi-word merchant name
    std::string id2_real = manager->generateOrderId("2026-11-15");
    TEST_ASSERT(id2_real == "10000215", "Second order ID sequentially increments to 10000215");
    bool created2 = manager->createOrder(id2_real, "2026-11-15", "11:00:00", "Jane Mary Smith",
                                         "", "on-hold", 120.50, "card", "Standard Chartered");
    TEST_ASSERT(created2 == true, "Order 2 created with empty optional contact and multi-word merchant");
    TEST_ASSERT(manager->getOrderCount() == 2, "Order count is 2");

    // --------------------------------------------------
    // Test 3: Adding Multiple Products
    // --------------------------------------------------
    std::cout << "\n--- Test 3: Adding Multiple Products ---" << std::endl;
    bool prod1 = manager->addProductToOrder(id1, "P101", "123456", 2);
    bool prod2 = manager->addProductToOrder(id1, "P102", "654321", 1);
    bool prod3 = manager->addProductToOrder(id1, "P103", "112233", 4);
    TEST_ASSERT(prod1 && prod2 && prod3, "Added 3 different products to Order 1");

    Order* foundOrder1 = manager->findOrder(id1);
    TEST_ASSERT(foundOrder1 != nullptr, "Found Order 1");
    TEST_ASSERT(foundOrder1->getDetailCount() == 3, "Order 1 contains 3 product detail entries");

    // --------------------------------------------------
    // Test 4: Quantity Increment on Duplicate Product Addition
    // --------------------------------------------------
    std::cout << "\n--- Test 4: Duplicate Product Addition (Quantity Increment) ---" << std::endl;
    bool prod1Again = manager->addProductToOrder(id1, "P101", "123456", 3);
    TEST_ASSERT(prod1Again == true, "Added duplicate product P101 again");
    TEST_ASSERT(foundOrder1->getDetailCount() == 3, "Detail count remains 3 (no duplicate entry)");
    int p101Idx = foundOrder1->findDetailIndex("P101");
    TEST_ASSERT(p101Idx != -1, "Found product P101 in details");
    TEST_ASSERT(foundOrder1->getDetail(p101Idx).getQuantity() == 5, "Product P101 quantity incremented to 5 (2 + 3)");

    // --------------------------------------------------
    // Test 5: Update Product Quantity Directly
    // --------------------------------------------------
    std::cout << "\n--- Test 5: Update Product Quantity ---" << std::endl;
    bool updatedQty = foundOrder1->updateProductQuantity("P102", 10);
    TEST_ASSERT(updatedQty == true, "Updated product P102 quantity to 10");
    int p102Idx = foundOrder1->findDetailIndex("P102");
    TEST_ASSERT(foundOrder1->getDetail(p102Idx).getQuantity() == 10, "Product P102 has updated quantity 10");

    delete manager;
    manager = nullptr;

    // --------------------------------------------------
    // Test 6: Verify Saved Order.txt Space-Separated & Quoted Format
    // --------------------------------------------------
    std::cout << "\n--- Test 6: Verify Saved Order.txt Format ---" << std::endl;
    std::ifstream oFile(testOrderFile);
    TEST_ASSERT(oFile.is_open(), "test_Order.txt was opened successfully");
    std::string line;
    int lineCount = 0;
    while (std::getline(oFile, line)) {
        if (!line.empty()) {
            lineCount++;
            std::cout << "Order record: " << line << std::endl;
            TEST_ASSERT(line.back() == '#', "Order record ends with '#'");
            TEST_ASSERT(line.find(',') == std::string::npos, "Record is space-separated without commas");
        }
    }
    oFile.close();
    TEST_ASSERT(lineCount == 2, "test_Order.txt contains exactly 2 order records");

    // --------------------------------------------------
    // Test 7: Verify Saved OrderDetail.txt Space-Separated Format
    // --------------------------------------------------
    std::cout << "\n--- Test 7: Verify Saved OrderDetail.txt Format ---" << std::endl;
    std::ifstream odFile(testDetailFile);
    TEST_ASSERT(odFile.is_open(), "test_OrderDetail.txt was opened successfully");
    int detailLineCount = 0;
    while (std::getline(odFile, line)) {
        if (!line.empty()) {
            detailLineCount++;
            std::cout << "Detail record: " << line << std::endl;
            TEST_ASSERT(line.back() == '#', "OrderDetail record ends with '#'");
            TEST_ASSERT(line.find(',') == std::string::npos, "OrderDetail record is space-separated without commas");
        }
    }
    odFile.close();
    TEST_ASSERT(detailLineCount == 3, "test_OrderDetail.txt contains exactly 3 product records");

    // --------------------------------------------------
    // Test 8: Loading Previously Saved Records
    // --------------------------------------------------
    std::cout << "\n--- Test 8: Loading from Files into New Manager ---" << std::endl;
    OrderManager* freshManager = new OrderManager(testOrderFile, testDetailFile);
    bool loaded = freshManager->loadAll();
    TEST_ASSERT(loaded == true, "loadAll() returned true");
    TEST_ASSERT(freshManager->getOrderCount() == 2, "Loaded exactly 2 orders");

    Order* loadedOrd1 = freshManager->findOrder(id1);
    TEST_ASSERT(loadedOrd1 != nullptr, "Loaded Order 1 found by Order ID");
    TEST_ASSERT(loadedOrd1->getCustomerName() == "John Doe", "Order 1 customer name correctly unquoted as 'John Doe'");
    TEST_ASSERT(loadedOrd1->getCustomerContact() == "03001234567", "Order 1 customer contact correctly loaded");
    TEST_ASSERT(loadedOrd1->getOrderStatus() == "in-progress", "Order 1 status is in-progress");
    TEST_ASSERT(loadedOrd1->getDetailCount() == 3, "Order 1 has 3 product details loaded");

    Order* loadedOrd2 = freshManager->findOrder(id2_real);
    TEST_ASSERT(loadedOrd2 != nullptr, "Loaded Order 2 found");
    TEST_ASSERT(loadedOrd2->getCustomerName() == "Jane Mary Smith", "Order 2 customer name correctly unquoted as 'Jane Mary Smith'");
    TEST_ASSERT(loadedOrd2->getCustomerContact() == "", "Order 2 empty optional contact correctly loaded as empty string");
    TEST_ASSERT(loadedOrd2->getPaymentMerchantName() == "Standard Chartered", "Order 2 merchant name correctly unquoted as 'Standard Chartered'");

    int loadedP101Idx = loadedOrd1->findDetailIndex("P101");
    TEST_ASSERT(loadedP101Idx != -1, "Order 1 contains P101");
    TEST_ASSERT(loadedOrd1->getDetail(loadedP101Idx).getQuantity() == 5, "Order 1 P101 has quantity 5");

    // --------------------------------------------------
    // Test 9: Update Order Status
    // --------------------------------------------------
    std::cout << "\n--- Test 9: Update Order Status ---" << std::endl;
    bool statusUpdated = freshManager->updateOrderStatus(id1, "paid");
    TEST_ASSERT(statusUpdated == true, "Updated Order 1 status to 'paid'");
    TEST_ASSERT(loadedOrd1->getOrderStatus() == "paid", "In-memory status is now 'paid'");

    // Verify persistence after status update
    OrderManager* verifyManager1 = new OrderManager(testOrderFile, testDetailFile);
    verifyManager1->loadOrders();
    Order* vOrd1 = verifyManager1->findOrder(id1);
    TEST_ASSERT(vOrd1 != nullptr && vOrd1->getOrderStatus() == "paid", "Status change persisted to file");
    delete verifyManager1;

    // --------------------------------------------------
    // Test 10: Update Payment Information
    // --------------------------------------------------
    std::cout << "\n--- Test 10: Update Payment Information ---" << std::endl;
    bool payUpdated = freshManager->updatePaymentInformation(id1, 450.75, "wallet", "Jazz Cash Wallet");
    TEST_ASSERT(payUpdated == true, "Updated payment info for Order 1 with multi-word merchant");
    TEST_ASSERT(loadedOrd1->getPaymentAmount() == 450.75, "In-memory payment amount updated");
    TEST_ASSERT(loadedOrd1->getPaymentMode() == "wallet", "In-memory payment mode updated to wallet");
    TEST_ASSERT(loadedOrd1->getPaymentMerchantName() == "Jazz Cash Wallet", "In-memory merchant updated to 'Jazz Cash Wallet'");

    // Verify persistence after payment update
    OrderManager* verifyManager2 = new OrderManager(testOrderFile, testDetailFile);
    verifyManager2->loadOrders();
    Order* vOrd2 = verifyManager2->findOrder(id1);
    TEST_ASSERT(vOrd2 != nullptr && vOrd2->getPaymentAmount() == 450.75, "Payment amount persisted to file");
    TEST_ASSERT(vOrd2 != nullptr && vOrd2->getPaymentMode() == "wallet", "Payment mode persisted to file");
    TEST_ASSERT(vOrd2 != nullptr && vOrd2->getPaymentMerchantName() == "Jazz Cash Wallet", "Payment merchant persisted as 'Jazz Cash Wallet'");

    // Verify unrelated Order 2 is untouched
    Order* vOrd2_2 = verifyManager2->findOrder(id2_real);
    TEST_ASSERT(vOrd2_2 != nullptr, "Unrelated Order 2 still exists");
    TEST_ASSERT(vOrd2_2->getCustomerName() == "Jane Mary Smith", "Order 2 customer name unchanged");
    TEST_ASSERT(vOrd2_2->getOrderStatus() == "on-hold", "Order 2 status unchanged");
    delete verifyManager2;

    delete freshManager;

    // --------------------------------------------------
    // Test 11: Missing and Empty File Handling
    // --------------------------------------------------
    std::cout << "\n--- Test 11: Missing and Empty File Handling ---" << std::endl;
    OrderManager* missingFilesManager = new OrderManager("non_existent_orders.txt", "non_existent_details.txt");
    bool loadMissing = missingFilesManager->loadAll();
    TEST_ASSERT(loadMissing == true, "Missing files handled safely without crash");
    TEST_ASSERT(missingFilesManager->getOrderCount() == 0, "Missing files result in 0 orders");
    delete missingFilesManager;

    // Cleanup test files
    std::remove(testOrderFile.c_str());
    std::remove(testDetailFile.c_str());

    std::cout << "\n=========================================" << std::endl;
    std::cout << "ALL 11 UNIT TESTS COMPLETED SUCCESSFULLY!" << std::endl;
    std::cout << "=========================================" << std::endl;
    return true;
}

int main() {
    bool success = runAllTests();
    return success ? 0 : 1;
}
