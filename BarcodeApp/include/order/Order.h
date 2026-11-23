#pragma once
#include <string>

// Maximum number of items allowed in a single order
const int MAX_DETAILS_PER_ORDER = 30;

// OrderDetail stores information about an individual product entry in an order
class OrderDetail {
private:
    std::string orderId;
    std::string productId;
    std::string productBarcode;
    int quantity;

public:
    // Default constructor
    OrderDetail();

    // Parameterized constructor
    OrderDetail(const std::string& orderId, const std::string& productId,
                const std::string& productBarcode, int quantity);

    // Getters
    std::string getOrderId() const;
    std::string getProductId() const;
    std::string getProductBarcode() const;
    int getQuantity() const;

    // Setters
    void setOrderId(const std::string& id);
    void setProductId(const std::string& id);
    void setProductBarcode(const std::string& barcode);
    void setQuantity(int qty);
};

// Order stores overall order information and its list of products
class Order {
private:
    std::string orderId;
    std::string orderDate;
    std::string orderTime;
    std::string customerName;
    std::string customerContact;
    std::string orderStatus;
    double paymentAmount;
    std::string paymentMode;
    std::string paymentMerchantName;
    std::string paymentDate; // Requirement 3: Dedicated payment date field

    // Array of details without using std::vector
    OrderDetail details[MAX_DETAILS_PER_ORDER];
    int detailCount;

public:
    // Default constructor
    Order();

    // Parameterized constructor (supports optional payment date)
    Order(const std::string& id, const std::string& date, const std::string& time,
          const std::string& custName, const std::string& custContact,
          const std::string& status, double amount,
          const std::string& payMode, const std::string& merchant,
          const std::string& payDate = "");

    // Getters
    std::string getOrderId() const;
    std::string getOrderDate() const;
    std::string getOrderTime() const;
    std::string getCustomerName() const;
    std::string getCustomerContact() const;
    std::string getOrderStatus() const;
    double getPaymentAmount() const;
    std::string getPaymentMode() const;
    std::string getPaymentMerchantName() const;
    std::string getPaymentDate() const;
    int getDetailCount() const;
    const OrderDetail& getDetail(int index) const;

    // Setters
    void setOrderId(const std::string& id);
    void setOrderDate(const std::string& date);
    void setOrderTime(const std::string& time);
    void setCustomerName(const std::string& name);
    void setCustomerContact(const std::string& contact);
    void setOrderStatus(const std::string& status);
    void setPaymentAmount(double amount);
    void setPaymentMode(const std::string& mode);
    void setPaymentMerchantName(const std::string& merchant);
    void setPaymentDate(const std::string& payDate);

    // Product item management
    bool addProduct(const std::string& productId, const std::string& productBarcode, int quantity);
    bool updateProductQuantity(const std::string& productId, int newQuantity);
    bool removeProduct(const std::string& productId);
    int findDetailIndex(const std::string& productId) const;
    int findDetailIndexByBarcode(const std::string& barcode) const;
    bool hasProduct(const std::string& productId) const;
    int getProductQuantity(const std::string& productId) const;
    void clearDetails();
};
