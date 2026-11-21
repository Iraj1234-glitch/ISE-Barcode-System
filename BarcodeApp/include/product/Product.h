#pragma once
#include <string>

// Represents an individual product in the Point of Sale system
class Product {
private:
    std::string productId;
    std::string barcode;
    std::string acronym;           // Increment 2: Product acronym (max 8 characters)
    std::string productName;
    std::string measurementUnit;   // Increment 2: Measurement unit ("count" or "weight")
    std::string description;
    double price;                  // Unit price
    double tax;                    // Increment 2: Tax on product in PKR
    double discount;               // Increment 2: Discount per unit
    int stockQuantity;             // Available quantity
    std::string barcodeImagePath;  // Increment 2: Barcode image filename and path
    std::string categoryId;        // Increment 3: two-digit category ID (Product.txt field 11)

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

    // Increment 2 full constructor
    Product(const std::string& id, const std::string& bCode,
            const std::string& acr, const std::string& name,
            const std::string& unit, double pr, double tx,
            double disc, int stock, const std::string& imgPath,
            const std::string& desc = "", const std::string& catId = "");

    // Getters
    std::string getProductId() const;
    std::string getBarcode() const;
    std::string getAcronym() const;
    std::string getProductName() const;
    std::string getMeasurementUnit() const;
    std::string getDescription() const;
    double getPrice() const;
    double getTax() const;
    double getDiscount() const;
    int getStockQuantity() const;
    std::string getBarcodeImagePath() const;
    std::string getCategoryId() const;

    // Setters
    void setProductId(const std::string& id);
    void setBarcode(const std::string& bCode);
    void setAcronym(const std::string& acr);
    void setProductName(const std::string& name);
    void setMeasurementUnit(const std::string& unit);
    void setDescription(const std::string& desc);
    void setPrice(double pr);
    void setTax(double tx);
    void setDiscount(double disc);
    void setStockQuantity(int stock);
    void setBarcodeImagePath(const std::string& path);
    void setCategoryId(const std::string& catId);

    // Stock operation methods
    bool isAvailable(int requestedQty) const;
    bool deductStock(int qty);
    bool addStock(int qty);

    // Formatted details suitable for GUI presentation
    std::string getFormattedDetails() const;
};
