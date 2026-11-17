#pragma once
#include <string>

// Represents an individual product in the Point of Sale system
class Product {
private:
    std::string productId;
    std::string barcode;
    std::string productName;
    std::string description;
    double price;
    int stockQuantity;

public:
    // Default constructor
    Product();

    // Parameterized constructor (standard 5 attributes, default stock 0)
    Product(const std::string& id, const std::string& bCode,
            const std::string& name, const std::string& desc,
            double pr);

    // Parameterized constructor including stock quantity
    Product(const std::string& id, const std::string& bCode,
            const std::string& name, const std::string& desc,
            double pr, int stock);

    // Getters
    std::string getProductId() const;
    std::string getBarcode() const;
    std::string getProductName() const;
    std::string getDescription() const;
    double getPrice() const;
    int getStockQuantity() const;

    // Setters
    void setProductId(const std::string& id);
    void setBarcode(const std::string& bCode);
    void setProductName(const std::string& name);
    void setDescription(const std::string& desc);
    void setPrice(double pr);
    void setStockQuantity(int stock);

    // Stock operation methods
    bool isAvailable(int requestedQty) const;
    bool deductStock(int qty);
    bool addStock(int qty);

    // Formatted details suitable for GUI presentation
    std::string getFormattedDetails() const;
};
