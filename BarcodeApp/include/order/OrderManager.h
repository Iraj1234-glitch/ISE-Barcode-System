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
    // Date & 7-day policy calculation helpers
    static bool parseDate(const std::string& dateStr, int& year, int& month, int& day);
    static int daysBetweenDates(const std::string& fromDate, const std::string& toDate);
    static std::string getCurrentSystemDate();
    static bool isWithinSevenDays(const std::string& paymentDate, const std::string& currentDate = "");

    // Constructors
    OrderManager();
    OrderManager(const std::string& orderFile, const std::string& orderDetailFile);

    // Order creation
    bool createOrder(const std::string& orderId, const std::string& orderDate,
                     const std::string& orderTime, const std::string& customerName,
                     const std::string& customerContact, const std::string& orderStatus,
                     double paymentAmount, const std::string& paymentMode,
                     const std::string& paymentMerchantName,
                     const std::string& paymentDate = "");

    // Order ID generation (8 digits, ending with 2-digit order date day)
    std::string generateOrderId(const std::string& orderDate) const;

    // Requirement 3: Order Editing, Manager Approval & 7-day Rule
    bool canEditOrder(const std::string& orderId, bool managerApproval = false,
                      const std::string& currentDate = "", std::string* outReason = nullptr) const;

    bool openOrder(const std::string& orderId, Order& outOrder) const;
    Order* openOrderForEditing(const std::string& orderId, bool managerApproval = false,
                               const std::string& currentDate = "", std::string* outReason = nullptr);

    bool addProductToOrder(const std::string& orderId, const std::string& productId,
                           const std::string& productBarcode, int quantity,
                           bool managerApproval = false, const std::string& currentDate = "");

    bool updateProductQuantityInOrder(const std::string& orderId, const std::string& productId,
                                      int newQuantity, bool managerApproval = false,
                                      const std::string& currentDate = "");

    bool removeProductFromOrder(const std::string& orderId, const std::string& productId,
                                bool managerApproval = false, const std::string& currentDate = "");

    // Requirement 3 & 10: On-hold order workflows
    bool holdOrder(const std::string& orderId);
    bool resumeOrder(const std::string& orderId);
    int getOnHoldOrders(Order outOrders[], int maxCapacity) const;
    int getOnHoldOrderCount() const;
    int getActiveOrderCount() const;
    int getPaidOrderCount() const;
    Order* getLatestOnHoldOrder();

    // Order lookup
    Order* findOrder(const std::string& orderId);
    const Order* findOrder(const std::string& orderId) const;

    // Updating orders
    bool updateOrderStatus(const std::string& orderId, const std::string& newStatus);
    bool updatePaymentInformation(const std::string& orderId, double paymentAmount,
                                  const std::string& paymentMode, const std::string& merchantName,
                                  const std::string& paymentDate = "");
    bool markOrderAsPaid(const std::string& orderId, double paymentAmount,
                         const std::string& paymentMode, const std::string& merchantName,
                         const std::string& paymentDate = "");

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
