#pragma once
#include "Order.h"
#include <string>

// Maximum number of orders stored in memory
const int MAX_ORDERS = 100;

class OrderManager {
private:
    Order orders[MAX_ORDERS];
    int orderCount;

    std::string orderFilePath;
    std::string orderDetailFilePath;

    // Helper functions
    bool isValidStatus(const std::string& status) const;
    bool isValidPaymentMode(const std::string& mode) const;
    bool isValidOrderId(const std::string& orderId, const std::string& orderDate) const;
    std::string extractDayFromDate(const std::string& date) const;

public:
    // Constructors
    OrderManager();
    OrderManager(const std::string& orderFile, const std::string& orderDetailFile);

    // Order creation
    bool createOrder(const std::string& orderId, const std::string& orderDate,
                     const std::string& orderTime, const std::string& customerName,
                     const std::string& customerContact, const std::string& orderStatus,
                     double paymentAmount, const std::string& paymentMode,
                     const std::string& paymentMerchantName);

    // Order ID generation (8 digits, ending with 2-digit order date day)
    std::string generateOrderId(const std::string& orderDate) const;

    // Product addition to an existing order
    bool addProductToOrder(const std::string& orderId, const std::string& productId,
                           const std::string& productBarcode, int quantity);

    // Order lookup
    Order* findOrder(const std::string& orderId);
    const Order* findOrder(const std::string& orderId) const;

    // Updating orders
    bool updateOrderStatus(const std::string& orderId, const std::string& newStatus);
    bool updatePaymentInformation(const std::string& orderId, double paymentAmount,
                                  const std::string& paymentMode, const std::string& merchantName);

    // File persistence
    bool loadOrders();
    bool loadOrderDetails();
    bool loadAll();

    bool saveOrder(const Order& order);
    bool saveOrderDetails(const Order& order);
    bool saveAllOrders();
    bool saveAllOrderDetails();
    bool saveAll();

    // Accessors
    int getOrderCount() const;
    const Order* getOrderAt(int index) const;
    Order* getOrderAt(int index);

    // Clear all orders in memory
    void clear();
};
